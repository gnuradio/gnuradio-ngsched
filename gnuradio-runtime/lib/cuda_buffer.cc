/* -*- c++ -*- */
/*
 * Copyright 2004,2009,2010,2013 Free Software Foundation, Inc.
 *
 * This file is part of GNU Radio
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 */

#include <gnuradio/cuda_buffer.h>

namespace gr {

cuda_buffer::cuda_buffer(int nitems,
                         size_t sizeof_item,
                         uint64_t downstream_lcm_nitems,
                         block_sptr link,
                         block_sptr buf_owner)
    : buffer_single_mapped(nitems,
                           sizeof_item,
                           downstream_lcm_nitems,
                           link,
                           buf_owner)
{
}

cuda_buffer::~cuda_buffer()
{
}

bool cuda_buffer::post_work(size_t nitems)
{
#if 0
    switch(get_buffer_context()) {
    case buffer_context::HOST_TO_DEVICE:
    {
        cudaMemcpy(d_device_buffer, d_host_buffer, ...);
        break;
    }
    case buffer_context::DEVICE_TO_HOST:
    {
        cudaMemcpy(d_host_buffer, d_device_buffer, ...);
        break;
    }
    case buffer_context::HOST_TO_HOST:
    case buffer_context::DEVICE_TO_DEVICE:
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
#endif

    return true;
}

buffer_sptr cuda_buffer::make_cuda_buffer(int nitems,
                                      size_t sizeof_item,
                                      uint64_t downstream_lcm_nitems,
                                      block_sptr link,
                                      block_sptr buf_owner)
{
    return buffer_sptr(new cuda_buffer(nitems, sizeof_item, downstream_lcm_nitems,
                                       link, buf_owner));
}

}
