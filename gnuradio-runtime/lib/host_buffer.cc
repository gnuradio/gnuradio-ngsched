/* -*- c++ -*- */
/*
 * Copyright 2004,2009,2010,2013 Free Software Foundation, Inc.
 *
 * This file is part of GNU Radio
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 */

// debugging
#include <iostream>
using std::endl;
using std::cout;

#include <gnuradio/buffer_single_mapped.h>


namespace gr {

host_buffer::host_buffer(BufferMappingType buf_type,
                         int nitems,
                         size_t sizeof_item,
                         uint64_t downstream_lcm_nitems,
                         block_sptr link)
    : buffer_single_mapped(buf_type,
                           nitems,
                           sizeof_item,
                           downstream_lcm_nitems,
                           link)
{
    cout << "**** host_buffer ctor" << endl;
}

bool hip_buffer::post_work(size_t nsize)
{
    switch(this->get_buffer_context()) {
    case BUFFER_CONTEXT_HOST_TO_DEVICE:
    {
        for (size_t i=0; i<nsize; ++i)
            d_device_buffer[i] = d_host_buffer[i];
        break;
    }
    case BUFFER_CONTEXT_DEVICE_TO_HOST:
    {
        for (size_t i=0; i<nsize; ++i)
            d_host_buffer[i] = d_device_buffer[i];
        break;
    }
    case BUFFER_CONTEXT_HOST_TO_HOST:
    case BUFFER_CONTEXT_DEVICE_TO_DEVICE:
    default:
    {
        // print warning message/throw exception/return false?
        std::ostringstream msg;
        msg << "[" << this << "] "
            << "hip_buffer::post_work() - invalid buffer context: "
            << this->get_buffer_context();
        GR_LOG_WARN(d_logger, msg.str());
        return false;
        break;
    }

    return true;
}

buffer* host_buffer::make_host_buffer(int nitems,
                                      size_t sizeof_item,
                                      uint64_t downstream_lcm_nitems,
                                      block_sptr link)
{
    return new host_buffer(nitems, sizeof_item, downstream_lcm_nitems, link);
}



}
