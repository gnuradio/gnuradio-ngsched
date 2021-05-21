/* -*- c++ -*- */
/*
 * Copyright 2004,2009,2010,2013 Free Software Foundation, Inc.
 *
 * This file is part of GNU Radio
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 */

#include <gnuradio/host_buffer.h>
#include <gnuradio/block.h>

#include <cstring>
#include <sstream>
#include <stdexcept>


namespace gr {

void* host_buffer::device_memcpy(void* dest, const void* src, std::size_t count)
{
    // There is no spoon...er... device so fake it out using regular memcpy
    return std::memcpy(dest, src, count);
}

void* host_buffer::device_memmove(void* dest, const void* src, std::size_t count)
{
    // There is no spoon...er... device so fake it out using regular memmmove
    return std::memmove(dest, src, count);
}


host_buffer::host_buffer(int nitems,
                         size_t sizeof_item,
                         uint64_t downstream_lcm_nitems,
                         block_sptr link,
                         block_sptr buf_owner)
    : buffer_single_mapped(nitems,
                           sizeof_item,
                           downstream_lcm_nitems,
                           link,
                           buf_owner),
    d_device_base(nullptr)    
{
    gr::configure_default_loggers(d_logger, d_debug_logger, "host_buffer");
    if (!allocate_buffer(nitems, sizeof_item, downstream_lcm_nitems))
        throw std::bad_alloc();
    
//#ifdef BUFFER_DEBUG
//    // BUFFER DEBUG
//    {
//        std::ostringstream msg;
//        msg << "[" << this << "] "
//            << "host_buffer constructor -- history: " << link->history()
//            << " -- nitems: " << nitems;
//        GR_LOG_DEBUG(d_logger, msg.str());
//    }
//#endif
}

host_buffer::~host_buffer()
{
}

bool host_buffer::post_work(int nitems)
{
#ifdef BUFFER_DEBUG
    std::ostringstream msg;
    msg << "[" << this << "] "
        << "host_buffer [" << d_context << "] -- post_work: " << nitems;
    GR_LOG_DEBUG(d_logger, msg.str());
#endif
    
    if (nitems <= 0)
    {
        return true;
    }
    
    // NOTE: when this function is called the write pointer has not yet been 
    // advanced so it can be used directly as the source ptr
    switch (d_context)
    {
        case buffer_context::HOST_TO_DEVICE:
        {
            // Copy data from host buffer to device buffer
            void* dest_ptr = &d_device_base[d_write_index * d_sizeof_item];
            device_memcpy(dest_ptr, write_pointer(), nitems * d_sizeof_item);
        }
            break;
            
        case buffer_context::DEVICE_TO_HOST:
        {
            // Copy data from device buffer to host buffer
            void* dest_ptr = &d_base[d_write_index * d_sizeof_item];
            device_memcpy(dest_ptr, write_pointer(), nitems * d_sizeof_item);
        }
            break;
        
        case buffer_context::DEVICE_TO_DEVICE:
            // No op
            break;
            
        default:
            std::ostringstream msg;
            msg << "Unexpected context for host_buffer: " << d_context;
            GR_LOG_ERROR(d_logger, msg.str());
            throw std::runtime_error(msg.str());
    }
    
    return true;
}
    
bool host_buffer::do_allocate_buffer(int final_nitems, size_t sizeof_item)
{
#ifdef BUFFER_DEBUG
    {
        std::ostringstream msg;
        msg << "[" << this << "] "
            << "host_buffer constructor -- nitems: " << final_nitems;
        GR_LOG_DEBUG(d_logger, msg.str());
    }
#endif
    
    // This is the host buffer
    d_buffer.reset(new char[final_nitems * sizeof_item]());
    d_base = d_buffer.get();
    
    // This is the simulated device buffer
    d_device_buf.reset(new char[final_nitems * sizeof_item]());
    d_device_base = d_device_buf.get();
    
    return true;
}

void* host_buffer::write_pointer()
{
    void* ptr = nullptr;
    switch (d_context)
    {
        case buffer_context::HOST_TO_DEVICE:
            // Write into host buffer
            ptr = &d_base[d_write_index * d_sizeof_item];
            break;
            
        case buffer_context::DEVICE_TO_HOST:
        case buffer_context::DEVICE_TO_DEVICE:
            // Write into "device" buffer
            ptr = &d_device_base[d_write_index * d_sizeof_item];
            break;
            
        default:
            std::ostringstream msg;
            msg << "Unexpected context for host_buffer: " << d_context;
            GR_LOG_ERROR(d_logger, msg.str());
            throw std::runtime_error(msg.str());
    }
    
    return ptr;
}

const void* host_buffer::_read_pointer(unsigned int read_index)
{
    void* ptr = nullptr;
    switch (d_context)
    {
        case buffer_context::HOST_TO_DEVICE:
        case buffer_context::DEVICE_TO_DEVICE:
            // Read from "device" buffer
            ptr = &d_device_base[read_index * d_sizeof_item];
            break;
            
        case buffer_context::DEVICE_TO_HOST:
            // Read from host buffer
            ptr = &d_base[read_index * d_sizeof_item];
            break;
            
        default:
            std::ostringstream msg;
            msg << "Unexpected context for host_buffer: " << d_context;
            GR_LOG_ERROR(d_logger, msg.str());
            throw std::runtime_error(msg.str());
    }
    
    return ptr;
}

bool host_buffer::input_blocked_callback(int items_required, int items_avail, 
                                         unsigned read_index)
{
#ifdef BUFFER_DEBUG
    std::ostringstream msg;
    msg << "[" << this << "] "
        << "host_buffer [" << d_context << "] -- input_blocked_callback";
    GR_LOG_DEBUG(d_logger, msg.str());
#endif
    
    bool rc = false;
    switch (d_context)
    {
        case buffer_context::HOST_TO_DEVICE:
        case buffer_context::DEVICE_TO_DEVICE:
            // Adjust "device" buffer
            rc = input_blocked_callback_logic(items_required, 
                                              items_avail,
                                              read_index,
                                              d_device_base,
                                              host_buffer::device_memcpy,
                                              host_buffer::device_memmove);
            break;
            
        case buffer_context::DEVICE_TO_HOST:
            // Adjust host buffer
            rc = input_blocked_callback_logic(items_required, 
                                              items_avail,
                                              read_index,
                                              d_base,
                                              std::memcpy,
                                              std::memmove);
            break;
            
        default:
            std::ostringstream msg;
            msg << "Unexpected context for host_buffer: " << d_context;
            GR_LOG_ERROR(d_logger, msg.str());
            throw std::runtime_error(msg.str());
    }
    
    return rc;
}

bool host_buffer::output_blocked_callback(int output_multiple, bool force)
{
#ifdef BUFFER_DEBUG
    std::ostringstream msg;
    msg << "[" << this << "] "
        << "host_buffer [" << d_context << "] -- output_blocked_callback";
    GR_LOG_DEBUG(d_logger, msg.str());
#endif
    
    bool rc = false;
    switch (d_context)
    {
        case buffer_context::HOST_TO_DEVICE:
            // Adjust host buffer
            rc = output_blocked_callback_logic(output_multiple, 
                                               force, 
                                               d_base, 
                                               std::memmove);
            break;
            
        case buffer_context::DEVICE_TO_HOST:
        case buffer_context::DEVICE_TO_DEVICE:
            // Adjust "device" buffer
            rc = output_blocked_callback_logic(output_multiple, 
                                               force, 
                                               d_device_base, 
                                               host_buffer::device_memmove);
            break;
            
        default:
            std::ostringstream msg;
            msg << "Unexpected context for host_buffer: " << d_context;
            GR_LOG_ERROR(d_logger, msg.str());
            throw std::runtime_error(msg.str());
    }
    
    return rc;
}

buffer_sptr host_buffer::make_host_buffer(int nitems,
                                          size_t sizeof_item,
                                          uint64_t downstream_lcm_nitems,
                                          block_sptr link,
                                          block_sptr buf_owner)
{
    return buffer_sptr(new host_buffer(nitems, sizeof_item, downstream_lcm_nitems,
                                       link, buf_owner));
}

}
