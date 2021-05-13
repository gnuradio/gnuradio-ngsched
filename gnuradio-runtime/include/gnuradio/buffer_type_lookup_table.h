/* -*- c++ -*- */
/*
 * Copyright 2012-2013 Free Software Foundation, Inc.
 *
 * This file is part of GNU Radio
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 */

#ifndef GR_RUNTIME_BUFFER_TYPE_LOOKUP_TABLE_H
#define GR_RUNTIME_BUFFER_TYPE_LOOKUP_TABLE_H

#include <gnuradio/buffer_type.h>
#include <gnuradio/buffer_context.h>
#include <gnuradio/custom_lock.h>
#include <gnuradio/thread/thread.h>
#include <gnuradio/cuda_buffer.h>
#include <gnuradio/hip_buffer.h>
#include <gnuradio/host_buffer.h>

#include <map>

/*!
 * \brief Thread-safe utility class used to map buffer_types to factory functions
 *
 */

namespace gr {

typedef buffer_sptr (*func_ptr_t)(int nitems,
                                 size_t sizeof_item,
                                 uint64_t downstrea_lcm_nitems,
                                 block_sptr link,
                                 block_sptr buf_owner);

class buffer_type_lookup_table
{
public:
    virtual ~buffer_type_lookup_table() {}

    /*!
     * \brief Returns the singleton object
     *
     */
    static buffer_type_lookup_table& get_instance();

    /*!
     * \brief Insert a new mapping (buffer_type -> func_ptr)
     *
     * \details
     *
     * \param bt   buffer_type_t (compile-time generated class)
     * \param fn   function pointer used to create the associated buffer
     * \return     bool: true if the operation was successful, false otherwise
     */
    //bool insert(buffer_type_t bt, func_ptr_t fn);
    bool insert(buffer_type_base* bt, func_ptr_t fn);

    /*!
     * \brief Return the function pointer asscociated with the given buffer_type
     *
     * \details
     *
     * \param bt   buffer_type_t compile-time-generated class instance
     * \return     factory function ptr used to create a buffer of type 'e'
     */
    func_ptr_t lookup(buffer_type_base* bt);

    /*!
     * \brief Removes the entry corresponding to 'e'
     *
     * \details
     *
     * \param bt   buffer_type enum
     * \return     bool: true if the operation was successful, false otherwise
     */
    bool erase(buffer_type_base* bt);

private:

    // private constructor
    buffer_type_lookup_table();

    // explicitly disallow copy ctor and assignment operator
    buffer_type_lookup_table(buffer_type_lookup_table const&) = delete; //!< copy ctor
    void operator=(buffer_type_lookup_table const&) = delete; //!< assignment operator

    // the map being wrapped
    typedef std::map<buffer_type_t, func_ptr_t> buffer_type_to_fn_ptr_map;
    static buffer_type_to_fn_ptr_map d_map;

    // mutex protecting operations on the map
    gr::thread::mutex d_mutex;
};

} /* namespace gr */

#endif /* GR_RUNTIME_BUFFER_TYPE_LOOKUP_TABLE_H */
