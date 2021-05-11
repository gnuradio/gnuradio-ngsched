/* -*- c++ -*- */
/*
 * Copyright 2020 Free Software Foundation, Inc.
 *
 * This file is part of GNU Radio
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 */

#ifndef INCLUDED_GR_RUNTIME_HOST_BUFFER_H
#define INCLUDED_GR_RUNTIME_HOST_BUFFER_H

#include <gnuradio/buffer_single_mapped.h>

namespace gr {

class GR_RUNTIME_API host_buffer : public buffer_single_mapped
{
public:
    
    virtual ~host_buffer();
    
    /*!
     * \brief Handle post-general_work() cleanup and data transfer
     *
     * Called directly after call to general_work() completes and
     * is used for data transfer (and perhaps other administrative
     * activities)
     *
     * \param nitems is the minimum number of items the buffer will hold.
     */
    bool post_work(size_t nitems);

    /*!
     * \brief Creates a new host_buffer object
     *
     * \param nitems
     * \param sizeof_item
     * \param downstream_lcm_nitems
     * \param link
     *
     * \return pointer to buffer base class
     */
    buffer* make_host_buffer(int nitems,
                             size_t sizeof_item,
                             uint64_t downstream_lcm_nitems,
                             block_sptr link);
    
private:
    /*!
     * \brief constructor is private.  Use gr_make_buffer to create instances.
     *
     * Allocate a buffer that holds at least \p nitems of size \p sizeof_item.
     *
     * \param nitems is the minimum number of items the buffer will hold.
     * \param sizeof_item is the size of an item in bytes.
     * \param downstream_lcm_nitems is the least common multiple of the items to
     *                              read by downstream blocks
     * \param link is the block that writes to this buffer.
     * \param buf_owner if the block that owns the buffer which may or may not
     *                  be the same as the block that writes to this buffer
     *
     * The total size of the buffer will be rounded up to a system
     * dependent boundary.  This is typically the system page size, but
     * under MS windows is 64KB.
     */
    host_buffer(int nitems,
                size_t sizeof_item,
                uint64_t downstream_lcm_nitems,
                block_sptr link,
                block_sptr buf_owner);

};

} /* namespace gr */

#endif /* INCLUDED_GR_RUNTIME_HOST_BUFFER_H */
