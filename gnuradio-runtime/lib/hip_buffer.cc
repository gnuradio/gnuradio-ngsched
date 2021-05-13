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

#include <gnuradio/hip_buffer.h>


namespace gr {

hip_buffer::hip_buffer(int nitems,
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
    cout << "**** hip_buffer ctor" << endl;
}

hip_buffer::~hip_buffer()
{
}

bool hip_buffer::post_work(size_t nsize)
{
#if 0
    switch(get_buffer_context()) {
    case buffer_context::HOST_TO_DEVICE:
    {
        hipMemcpy(d_device_buffer, d_host_buffer, ...);
        break;
    }
    case buffer_context::DEVICE_TO_HOST:
    {
        hipMemcpy(d_host_buffer, d_device_buffer, ...);
        break;
    }
    case buffer_context::HOST_TO_HOST:
    case buffer_context::DEVICE_TO_DEVICE:
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
#endif
    return true;
}
    
bool hip_buffer::do_allocate_buffer(int final_nitems, size_t sizeof_item)
{
    d_buffer.reset(new char[final_nitems * sizeof_item]);
    return true;
}

buffer_sptr hip_buffer::make_hip_buffer(int nitems,
                                    size_t sizeof_item,
                                    uint64_t downstream_lcm_nitems,
                                    block_sptr link,
                                    block_sptr buf_owner)
{
    return buffer_sptr(new hip_buffer(nitems, sizeof_item, downstream_lcm_nitems,
                                      link, buf_owner));
}

}
