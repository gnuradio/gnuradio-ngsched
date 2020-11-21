/* -*- c++ -*- */
/*
 * Copyright 2004,2009,2010,2013 Free Software Foundation, Inc.
 *
 * This file is part of GNU Radio
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include "vmcircbuf.h"
#include <gnuradio/buffer.h>
#include <gnuradio/buffer_double_mapped.h>
#include <gnuradio/buffer_reader.h>
#include <gnuradio/buffer_single_mapped.h>
#include <gnuradio/buffer_type.h>
#include <gnuradio/integer_math.h>
#include <gnuradio/math.h>
#include <assert.h>
#include <algorithm>
#include <iostream>
#include <stdexcept>

namespace gr {

static long s_buffer_count = 0; // counts for debugging storage mgmt

/* ----------------------------------------------------------------------------
                      Notes on storage management

 Pretty much all the fundamental classes are now using the
 shared_ptr stuff for automatic reference counting. To ensure that
 no mistakes are made, we make the constructors for classes private,
 and then provide a free factory function that returns a smart
 pointer to the desired class.

 gr::buffer and gr::buffer_reader are no exceptions.  However, they
 both want pointers to each other, and unless we do something, we'll
 never delete any of them because of the circular structure.
 They'll always have a reference count of at least one.  We could
 use std::weak_ptr's from gr::buffer to gr::buffer_reader but that
 introduces it's own problems.  (gr::buffer_reader's destructor needs
 to call gr::buffer::drop_reader, but has no easy way to get a
 shared_ptr to itself.)

 Instead, we solve this problem by having gr::buffer hold a raw
 pointer to gr::buffer_reader in its d_reader vector.
 gr::buffer_reader's destructor calls gr::buffer::drop_reader, so
 we're never left with an dangling pointer.  gr::buffer_reader still
 has a shared_ptr to the buffer ensuring that the buffer doesn't go
 away under it.  However, when the reference count of a
 gr::buffer_reader goes to zero, we can successfully reclaim it.
 ---------------------------------------------------------------------------- */


buffer::buffer(BufferMappingType buf_type,
               int nitems,
               size_t sizeof_item,
               uint64_t downstream_lcm_nitems,
               block_sptr link)
    : d_base(0),
      d_bufsize(0),
      d_buf_map_type(buf_type),
      d_max_reader_delay(0),
      d_max_reader_history(1),
      d_has_history(false),
      d_sizeof_item(sizeof_item),
      d_link(link),
      d_write_index(0),
      d_abs_write_offset(0),
      d_done(false),
      d_last_min_items_read(0),
      d_downstream_lcm_nitems(downstream_lcm_nitems),
      d_write_multiple(0)
{
    gr::configure_default_loggers(d_logger, d_debug_logger, "buffer");

    s_buffer_count++;
}

buffer_sptr make_buffer(int nitems,
                        size_t sizeof_item,
                        uint64_t downstream_lcm_nitems,
                        block_sptr link,
                        block_sptr buf_owner)
{
#ifdef BUFFER_DEBUG
    // BUFFER DEBUG
    gr::logger_ptr logger;
    gr::logger_ptr debug_logger;
    gr::configure_default_loggers(logger, debug_logger, "make_buffer");
    std::ostringstream msg;
#endif

    if (buf_owner->get_buffer_type() == buftype_DEFAULT_NON_CUSTOM::get()) {
#ifdef BUFFER_DEBUG
        msg << "buffer_double_mapped nitems: " << nitems
            << " -- sizeof_item: " << sizeof_item;
        GR_LOG_DEBUG(logger, msg.str());
#endif

        return buffer_sptr(
            new buffer_double_mapped(nitems, sizeof_item, downstream_lcm_nitems, link));
    } else {
#ifdef BUFFER_DEBUG
        msg << "buffer_single_mapped nitems: " << nitems
            << " -- sizeof_item: " << sizeof_item;
        GR_LOG_DEBUG(logger, msg.str());
#endif

        return buffer_sptr(new buffer_single_mapped(
            nitems, sizeof_item, downstream_lcm_nitems, link, buf_owner));
    }
}

buffer::~buffer()
{
    assert(d_readers.size() == 0);
    s_buffer_count--;
}

void* buffer::write_pointer() { return &d_base[d_write_index * d_sizeof_item]; }

void buffer::update_write_pointer(int nitems)
{
    gr::thread::scoped_lock guard(*mutex());

#ifdef BUFFER_DEBUG
    // BUFFER DEBUG
    unsigned orig_wr_idx = d_write_index;
#endif

    d_write_index = index_add(d_write_index, nitems);
    d_abs_write_offset += nitems;

#ifdef BUFFER_DEBUG
    // BUFFER DEBUG
    std::ostringstream msg;
    msg << "[" << this << "] update_write_pointer -- orig d_write_index: " << orig_wr_idx
        << " -- nitems: " << nitems << " -- d_write_index: " << d_write_index;
    GR_LOG_DEBUG(d_logger, msg.str());
#endif
}

void buffer::set_done(bool done)
{
    gr::thread::scoped_lock guard(*mutex());
    d_done = done;
}

void buffer::drop_reader(buffer_reader* reader)
{
    std::vector<buffer_reader*>::iterator result =
        std::find(d_readers.begin(), d_readers.end(), reader);

    if (result == d_readers.end())
        throw std::invalid_argument("buffer::drop_reader"); // we didn't find it...

    d_readers.erase(result);
}

void buffer::add_item_tag(const tag_t& tag)
{
    gr::thread::scoped_lock guard(*mutex());
    d_item_tags.insert(std::pair<uint64_t, tag_t>(tag.offset, tag));
}

void buffer::remove_item_tag(const tag_t& tag, long id)
{
    gr::thread::scoped_lock guard(*mutex());
    for (std::multimap<uint64_t, tag_t>::iterator it =
             d_item_tags.lower_bound(tag.offset);
         it != d_item_tags.upper_bound(tag.offset);
         ++it) {
        if ((*it).second == tag) {
            (*it).second.marked_deleted.push_back(id);
        }
    }
}

void buffer::prune_tags(uint64_t max_time)
{
    /* NOTE: this function _should_ lock the mutex before editing
       d_item_tags. In practice, this function is only called at
       runtime by min_available_space in block_executor.cc, which
       locks the mutex itself.

       If this function is used elsewhere, remember to lock the
       buffer's mutex al la the scoped_lock:
           gr::thread::scoped_lock guard(*mutex());
     */

    /*
      http://www.cplusplus.com/reference/map/multimap/erase/
      "Iterators, pointers and references referring to elements removed
      by the function are invalidated. All other iterators, pointers
      and references keep their validity."

      Store the iterator to be deleted in tmp; increment itr to the
      next valid iterator, then erase tmp, which now becomes invalid.
     */

    uint64_t item_time;
    std::multimap<uint64_t, tag_t>::iterator itr(d_item_tags.begin()), tmp;
    while (itr != d_item_tags.end()) {
        item_time = (*itr).second.offset;
        if (item_time + d_max_reader_delay + bufsize() < max_time) {
            tmp = itr;
            itr++;
            d_item_tags.erase(tmp);
        } else {
            // d_item_tags is a map sorted by offset, so when the if
            // condition above fails, all future tags in the map must also
            // fail. So just break here.
            break;
        }
    }
}

long buffer_ncurrently_allocated() { return s_buffer_count; }

} /* namespace gr */
