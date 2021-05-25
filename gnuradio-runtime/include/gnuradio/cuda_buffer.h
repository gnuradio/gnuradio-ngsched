/* -*- c++ -*- */
/*
 * Copyright 2020 Free Software Foundation, Inc.
 *
 * This file is part of GNU Radio
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 */

#ifndef INCLUDED_GR_RUNTIME_CUDA_BUFFER_H
#define INCLUDED_GR_RUNTIME_CUDA_BUFFER_H

#include <gnuradio/buffer_type.h>
#include <gnuradio/buffer_single_mapped.h>

#include <cuda.h>
#include <cuda_runtime_api.h>

namespace gr {

class GR_RUNTIME_API cuda_buffer : public buffer_single_mapped
{
  public:
      
    static void* cuda_memcpy(void* dest, const void* src, std::size_t count);
    static void* cuda_memmove(void* dest, const void* src, std::size_t count);

    virtual ~cuda_buffer();

    /*!
     * \brief Handle post-general_work() cleanup and data transfer
     *
     * Called directly after call to general_work() completes and
     * is used for data transfer (and perhaps other administrative
     * activities)
     *
     * \param nitems is the minimum number of items the buffer will hold.
     */
    virtual bool post_work(int nitems);

    /*!
     * \brief Do actual buffer allocation. Inherited from buffer_single_mapped.
     */
    bool do_allocate_buffer(int final_nitems, size_t sizeof_item);

    /*!
     * \brief Return a pointer to the write buffer depending on the context
     */
    virtual void* write_pointer();
    
    /*!
     * \brief return pointer to read buffer depending on the context
     * 
     * The return value points to at least items_available() items.
     */
    virtual const void* _read_pointer(unsigned int read_index);
    
    /*!
     * \brief Callback function that the scheduler will call when it determines
     * that the input is blocked. Override this function if needed.
     */
    bool input_blocked_callback(int items_required, int items_avail, 
                                unsigned read_index);
    
    /*!
     * \brief Callback function that the scheduler will call when it determines
     * that the output is blocked
     */
    bool output_blocked_callback(int output_multiple, bool force);
    
    /*!
     * \brief Creates a new cuda_buffer object
     *
     * \param nitems
     * \param sizeof_item
     * \param downstream_lcm_nitems
     * \param link
     * \param buf_owner
     *
     * \return pointer to buffer base class
     */
    static buffer_sptr make_cuda_buffer(int nitems,
                                 size_t sizeof_item,
                                 uint64_t downstream_lcm_nitems,
                                 block_sptr link,
                                 block_sptr buf_owner);
private:
    
    char* d_cuda_buf;   // CUDA buffer
    
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
    cuda_buffer(int nitems,
                size_t sizeof_item,
                uint64_t downstream_lcm_nitems,
                block_sptr link,
                block_sptr buf_owner);

};

// see buffer_type.h for details on this macro
// here we're using the macro to generate compile-time
// class declarations
MAKE_CUSTOM_BUFFER_TYPE(DEFAULT_CUDA, &cuda_buffer::make_cuda_buffer)

} /* namespace gr */

#endif /* INCLUDED_GR_RUNTIME_CUDA_BUFFER_H */
