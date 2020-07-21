/* -*- c++ -*- */
/*
 * Copyright 2020 Free Software Foundation, Inc.
 *
 * This file is part of GNU Radio
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include <gnuradio/block.h>
#include <gnuradio/buffer_single_mapped.h>
#include <gnuradio/buffer_reader.h>
#include <gnuradio/integer_math.h>
#include <gnuradio/math.h>
#include <assert.h>
#include <algorithm>
#include <iostream>
#include <stdexcept>

namespace gr {

buffer_single_mapped::buffer_single_mapped(int nitems, size_t sizeof_item, 
                                           uint64_t downstream_lcm_nitems, 
                                           block_sptr link, block_sptr buf_owner)
    : buffer(BufferMappingType::SingleMapped, nitems, sizeof_item, downstream_lcm_nitems, 
             link),
      d_buf_owner(buf_owner),
      d_buffer(nullptr, std::bind(&buffer_single_mapped::deleter, this, std::placeholders::_1))
{
    gr::configure_default_loggers(d_logger, d_debug_logger, "buffer_single_mapped");
    if (!allocate_buffer(nitems, sizeof_item, downstream_lcm_nitems))
        throw std::bad_alloc();
    
#ifdef BUFFER_DEBUG
    // BUFFER DEBUG
    {
        std::ostringstream msg;
        msg << "[" << this << "] " 
            << "buffer_single_mapped constructor -- history: " << link->history();
        GR_LOG_DEBUG(d_logger, msg.str());
    }
#endif
}

buffer_single_mapped::~buffer_single_mapped()
{
}

/*!
 * Allocates underlying buffer.
 * returns true iff successful.
 */
bool buffer_single_mapped::allocate_buffer(int nitems, size_t sizeof_item,
                                           uint64_t downstream_lcm_nitems)
{
    int orig_nitems = nitems;
    
    // Unlike the double mapped buffer case that can easily wrap back onto itself 
    // for both reads and writes the single mapped case needs to be aware of read
    // and write granularity and size the underlying buffer accordingly. Otherwise
    // the calls to space_available() and items_available() may return values that
    // are too small and the scheduler will get stuck.
    uint64_t write_granularity = 1;
    uint64_t lcm_granularity = 1;
    
    if (link()->relative_rate() != 1.0)
    {
        write_granularity = link()->relative_rate_i();
    }
    write_granularity = GR_LCM(link()->relative_rate_i(), 
                               (uint64_t)link()->output_multiple());
    
#ifdef BUFFER_DEBUG
    std::ostringstream msg;
    msg << "WRITE GRANULARITY: " << write_granularity;
    GR_LOG_DEBUG(d_logger, msg.str());
#endif
    
    
    // Determine the LCM of the write and read granularity then use it to adjust 
    // the size of the buffer so that it is a multiple of the LCM value
    if (write_granularity != 1 || downstream_lcm_nitems != 1)
    {
        lcm_granularity = GR_LCM(write_granularity, downstream_lcm_nitems);
        
        uint64_t remainder = nitems % lcm_granularity;
        nitems += (remainder > 0) ? (lcm_granularity - remainder) : 0;
        
#ifdef BUFFER_DEBUG
        std::ostringstream msg;
        msg << "allocate_buffer() called  nitems: " << orig_nitems 
            << " -- read_multiple: " << downstream_lcm_nitems 
            << " -- write_multiple: " << write_granularity 
            << " -- lcm_multiple: " << lcm_granularity
            << " -- NEW nitems: " << nitems;
        GR_LOG_DEBUG(d_logger, msg.str());
#endif
    }
    
    // Allocate a new custom buffer from the owning block
    char* buf = buf_owner()->allocate_custom_buffer(nitems * sizeof_item);
    assert(buf != nullptr);
    d_buffer.reset(buf);

    d_base = d_buffer.get();
    d_bufsize = nitems;
    
    d_downstream_lcm_nitems = downstream_lcm_nitems;
    d_write_multiple = write_granularity;
    
    return true;
}

int buffer_single_mapped::space_available()
{
    if (d_readers.empty())
        return d_bufsize - 1; // See comment below

    else {
        
        size_t min_read_index_idx = 0;
        size_t min_items_read_idx = 0;
        uint64_t min_items_read = d_readers[0]->nitems_read();
        for (size_t idx = 1; idx < d_readers.size(); ++idx) {
            // Record index of reader with minimum read-index
            if (d_readers[idx]->d_read_index < d_readers[min_read_index_idx]->d_read_index) {
                min_read_index_idx = idx;
            }
            
            // Record index of reader with minimum nitems read
            if (d_readers[idx]->nitems_read() < d_readers[min_items_read_idx]->d_read_index) {
                min_items_read_idx = idx;
            }
            min_items_read = std::min(min_items_read, d_readers[idx]->nitems_read());
        }
        
        buffer_reader* min_idx_reader = d_readers[min_items_read_idx];
        unsigned min_read_index = d_readers[min_items_read_idx]->d_read_index;
        
        // For single mapped buffer there is no wrapping beyond the end of the
        // buffer
        int thecase  = -1;  // REMOVE ME - just for debug
        int space = d_bufsize - d_write_index;
        
        if (nitems_written() > 0 && d_has_history && 
            (space < d_write_multiple || d_write_index == 0))
        {
            std::ostringstream msg;
            if (min_read_index > (d_downstream_lcm_nitems - 1))
            {
                // Copy last max d_downstream_lcm_nitems - 1 samples back to the 
                // beginning of the buffer
                size_t bytes_to_copy = (d_downstream_lcm_nitems - 1) * d_sizeof_item;
                char* src_ptr = d_base;                
                if (d_write_index == 0)
                {
                    src_ptr += ((d_bufsize - (d_downstream_lcm_nitems - 1)) * d_sizeof_item);
                }
                else
                {
                    src_ptr += ((d_bufsize - (d_downstream_lcm_nitems - 1 ) - space) * d_sizeof_item);
                }
                std::memcpy(d_base, src_ptr, bytes_to_copy);
                
                d_write_index = d_downstream_lcm_nitems - 1;
                space = (min_read_index - (d_downstream_lcm_nitems - 1)) - d_write_index;
                
#ifdef BUFFER_DEBUG
                msg << "[" << this << "] " 
                    << " RELOCATING d_write_index: "  << d_write_index 
                    << " -- min_read_index: " << min_read_index 
                    << " -- dstream_lcm_nitems: " << (d_downstream_lcm_nitems - 1)
                    << " -- space: " << space;
                GR_LOG_DEBUG(d_logger, msg.str());
                thecase = 18;
#endif
            }
            else
            {
                space = 0;
                
#ifdef BUFFER_DEBUG
                msg << "[" << this << "] " 
                    << " WAITING d_write_index: "  << d_write_index 
                    << " -- min_read_index: " << min_read_index 
                    << " -- dstream_lcm_nitems: " << (d_downstream_lcm_nitems - 1)
                    << " -- space: " << space;
                GR_LOG_DEBUG(d_logger, msg.str());
                thecase = 19;
#endif
            }
        }
        else if (min_read_index == d_write_index)
        {
            thecase = 1; 
            
            // If the (min) read index and write index are equal then the buffer
            // is either completely empty or completely full depending on if 
            // the number of items read matches the number written
            size_t offset = ((d_max_reader_history - 1) + min_idx_reader->sample_delay());
            if ((min_idx_reader->nitems_read() - offset) != nitems_written())
            {
                thecase = 2; 
                space = 0;
            }
        }
        else if (min_read_index > d_write_index)
        {
            thecase = 3;
            space = min_read_index - d_write_index;
        }
        
        if (min_items_read != d_last_min_items_read) {
            prune_tags(d_last_min_items_read);
            d_last_min_items_read = min_items_read;
        }
        
#ifdef BUFFER_DEBUG
        // BUFFER DEBUG
        std::ostringstream msg;
        msg << "[" << this << "] " 
            << "space_available() called  (case: " << thecase 
            << ")  d_write_index: "  << d_write_index << " (" << nitems_written() << ") "
            << " -- min_read_index: " << min_read_index << " (" << min_idx_reader->nitems_read() << ") "
            << " -- space: " << space << " (sample delay: " << min_idx_reader->sample_delay() << ")";
        GR_LOG_DEBUG(d_logger, msg.str());
#endif
        
        return space;
    }
}

} /* namespace gr */