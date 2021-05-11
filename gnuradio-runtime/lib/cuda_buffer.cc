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

#include <gnuradio/cuda_buffer.h>

namespace gr {

cuda_buffer::cuda_buffer(BufferMappingType buf_type,
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
    cout << "**** cuda_buffer ctor" << endl;
}

bool cuda_buffer::post_work(size_t nitems)
{
    switch(this->get_buffer_context()) {
    case BUFFER_CONTEXT_HOST_TO_DEVICE:
    {
        cudaMemcpy(d_device_buffer, d_host_buffer, ...);
        break;
    }
    case BUFFER_CONTEXT_DEVICE_TO_HOST:
    {
        cudaMemcpy(d_host_buffer, d_device_buffer, ...);
        break;
    }
    case BUFFER_CONTEXT_HOST_TO_HOST:
    case BUFFER_CONTEXT_DEVICE_TO_DEVICE:
    default:
    {
        // print warning message/throw exception/return false?
        std::ostringstream msg;
        msg << "[" << this << "] "
            << "cuda_buffer::post_work() - invalid buffer context: "
            << this->get_buffer_context();
        GR_LOG_WARN(d_logger, msg.str());
        return false;
        break;
    }

    return true;
}

buffer* cuda_buffer::make_host_buffer(int nitems,
                                      size_t sizeof_item,
                                      uint64_t downstream_lcm_nitems,
                                      block_sptr link)
{
    return new cuda_buffer(nitems, sizeof_item, downstream_lcm_nitems, link);
}

}
