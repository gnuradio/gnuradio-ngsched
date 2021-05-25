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
#include <gnuradio/block.h>

#include <cstring>
#include <sstream>
#include <stdexcept>

namespace gr {

void* cuda_buffer::cuda_memcpy(void* dest, const void* src, std::size_t count)
{
    cudaError_t rc = cudaSuccess;
    rc = cudaMemcpy(dest, src, count, cudaMemcpyDeviceToDevice);
    if (rc)
    {
        std::ostringstream msg;
        msg << "Error performing cudaMemcpy: " << cudaGetErrorName(rc) 
            << " -- " << cudaGetErrorString(rc);
        throw std::runtime_error(msg.str());
    }
    
    return dest;
}

void* cuda_buffer::cuda_memmove(void* dest, const void* src, std::size_t count)
{
    // TODO: memmove will have to be implemented manually using memcpy and, 
    //       unfortunately, a separate buffer
    assert(0);
    return nullptr;
}

cuda_buffer::cuda_buffer(int nitems,
                         size_t sizeof_item,
                         uint64_t downstream_lcm_nitems,
                         block_sptr link,
                         block_sptr buf_owner)
    : buffer_single_mapped(nitems,
                           sizeof_item,
                           downstream_lcm_nitems,
                           link,
                           buf_owner),
      d_cuda_buf(nullptr)
{
    gr::configure_default_loggers(d_logger, d_debug_logger, "cuda_buffer");
    if (!allocate_buffer(nitems, sizeof_item, downstream_lcm_nitems))
        throw std::bad_alloc();
}

cuda_buffer::~cuda_buffer()
{
    // Free host buffer
    if (d_base != nullptr)
    {
        cudaFreeHost(d_base);
        d_base = nullptr;
    }
    
    // Free device buffer
    if (d_cuda_buf != nullptr)
    {
        cudaFree(d_cuda_buf);
        d_cuda_buf = nullptr;
    }
}

bool cuda_buffer::post_work(int nitems)
{
#ifdef BUFFER_DEBUG
    std::ostringstream msg;
    msg << "[" << this << "] "
        << "cuda_buffer [" << d_context << "] -- post_work: " << nitems;
    GR_LOG_DEBUG(d_logger, msg.str());
#endif
    
    if (nitems <= 0)
    {
        return true;
    }
    
    cudaError_t rc = cudaSuccess;
    
    // NOTE: when this function is called the write pointer has not yet been 
    // advanced so it can be used directly as the source ptr
    switch (d_context)
    {
        case buffer_context::HOST_TO_DEVICE:
        {
            // Copy data from host buffer to device buffer
            void* dest_ptr = &d_cuda_buf[d_write_index * d_sizeof_item];
            rc = cudaMemcpy(dest_ptr, write_pointer(), nitems * d_sizeof_item, 
                            cudaMemcpyHostToDevice);
            if (rc)
            {
                std::ostringstream msg;
                msg << "Error performing cudaMemcpy: " << cudaGetErrorName(rc) 
                    << " -- " << cudaGetErrorString(rc);
                GR_LOG_ERROR(d_logger, msg.str());
                throw std::runtime_error(msg.str());
            }
        }
            break;
            
        case buffer_context::DEVICE_TO_HOST:
        {
            // Copy data from device buffer to host buffer
            void* dest_ptr = &d_base[d_write_index * d_sizeof_item];
            rc = cudaMemcpy(dest_ptr, write_pointer(), nitems * d_sizeof_item, 
                            cudaMemcpyDeviceToHost);
            if (rc)
            {
                std::ostringstream msg;
                msg << "Error performing cudaMemcpy: " << cudaGetErrorName(rc) 
                    << " -- " << cudaGetErrorString(rc);
                GR_LOG_ERROR(d_logger, msg.str());
                throw std::runtime_error(msg.str());
            }
        }
            break;
        
        case buffer_context::DEVICE_TO_DEVICE:
            // No op FTW!
            break;
            
        default:
            std::ostringstream msg;
            msg << "Unexpected context for cuda_buffer: " << d_context;
            GR_LOG_ERROR(d_logger, msg.str());
            throw std::runtime_error(msg.str());
    }
    
    return true;
}

bool cuda_buffer::do_allocate_buffer(int final_nitems, size_t sizeof_item)
{
#ifdef BUFFER_DEBUG
    {
        std::ostringstream msg;
        msg << "[" << this << "] "
            << "cuda_buffer constructor -- nitems: " << final_nitems;
        GR_LOG_DEBUG(d_logger, msg.str());
    }
#endif
    
    // This is the pinned host buffer
    // Can a CUDA buffer even use std::unique_ptr ?
//    d_buffer.reset(new char[final_nitems * sizeof_item]);
    cudaError_t rc = cudaSuccess;
    rc = cudaMallocHost((void**)&d_base, final_nitems * sizeof_item);
    if (rc)
    {
        std::ostringstream msg;
        msg << "Error allocating pinned host buffer: " << cudaGetErrorName(rc) 
            << " -- " << cudaGetErrorString(rc);
        GR_LOG_ERROR(d_logger, msg.str());
        throw std::runtime_error(msg.str());
    }
    
    // This is the CUDA device buffer
    rc = cudaMalloc((void**)&d_cuda_buf, final_nitems * sizeof_item);
    if (rc)
    {
        std::ostringstream msg;
        msg << "Error allocating device buffer: " << cudaGetErrorName(rc) 
            << " -- " << cudaGetErrorString(rc);
        GR_LOG_ERROR(d_logger, msg.str());
        throw std::runtime_error(msg.str());
    }
    
    return true;
}

void* cuda_buffer::write_pointer()
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
            // Write into CUDA device buffer
            ptr = &d_cuda_buf[d_write_index * d_sizeof_item];
            break;
            
        default:
            std::ostringstream msg;
            msg << "Unexpected context for cuda_buffer: " << d_context;
            GR_LOG_ERROR(d_logger, msg.str());
            throw std::runtime_error(msg.str());
    }
    
    return ptr;
}

const void* cuda_buffer::_read_pointer(unsigned int read_index)
{
    void* ptr = nullptr;
    switch (d_context)
    {
        case buffer_context::HOST_TO_DEVICE:
        case buffer_context::DEVICE_TO_DEVICE:
            // Read from "device" buffer
            ptr = &d_cuda_buf[read_index * d_sizeof_item];
            break;
            
        case buffer_context::DEVICE_TO_HOST:
            // Read from host buffer
            ptr = &d_base[read_index * d_sizeof_item];
            break;
            
        default:
            std::ostringstream msg;
            msg << "Unexpected context for cuda_buffer: " << d_context;
            GR_LOG_ERROR(d_logger, msg.str());
            throw std::runtime_error(msg.str());
    }
    
    return ptr;
}

bool cuda_buffer::input_blocked_callback(int items_required, int items_avail, 
                                         unsigned read_index)
{
#ifdef BUFFER_DEBUG
    std::ostringstream msg;
    msg << "[" << this << "] "
        << "cuda_buffer [" << d_context << "] -- input_blocked_callback";
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
                                              d_cuda_buf,
                                              cuda_buffer::cuda_memcpy,
                                              cuda_buffer::cuda_memmove);
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
            msg << "Unexpected context for cuda_buffer: " << d_context;
            GR_LOG_ERROR(d_logger, msg.str());
            throw std::runtime_error(msg.str());
    }
    
    return rc;
}

bool cuda_buffer::output_blocked_callback(int output_multiple, bool force)
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
                                               d_cuda_buf, 
                                               cuda_buffer::cuda_memmove);
            break;
            
        default:
            std::ostringstream msg;
            msg << "Unexpected context for cuda_buffer: " << d_context;
            GR_LOG_ERROR(d_logger, msg.str());
            throw std::runtime_error(msg.str());
    }
    
    return rc;
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
