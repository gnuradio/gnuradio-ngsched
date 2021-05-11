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

#include <map>
using std::map;

#include <gnuradio/custom_lock.h>
#include <gnuradio/thread/thread.h>
#include <gnuradio/cuda_buffer.h>
#include <gnuradio/hip_buffer.h>
#include <gnuradio/host_buffer.h>

#include <iostream> // debug
using std::cout;
using std::endl;
#include <mutex>
using std::mutex;
using std::unique_lock;

/*!
 * \brief Thread-safe utility class used to map buffer contexts to factory functions
 *
 */

namespace gr {

typedef int (*func_ptr_t)(int);

enum class buffer_type_t {
    BUFFER_TYPE_CUDA,
    BUFFER_TYPE_HIP,
    BUFFER_TYPE_HOST,
};

class buffer_type_lookup_table {

private:
    // private constructor
    buffer_type_lookup_table() {
        // here is where we need to populate the table with the
        // latest mappings ...
        d_map[BUFFER_TYPE_CUDA] = cuda_buffer::make_cuda_buffer;
        d_map[BUFFER_TYPE_HIP]  = hip_buffer::make_hip_buffer;
        d_map[BUFFER_TYPE_HOST] = host_buffer::make_host_buffer;
        // add more ??
    }

    // explicitly disallow copy ctor and assignment operator
    buffer_type_lookup_table(buffer_type_lookup_table const&) = delete; //!< copy ctor
    void operator=(buffer_type_lookup_table const&) = delete; //!< assignment operator

    // the map being wrapped
    std::map<buffer_context_enum, func_ptr_t> d_map;

    // mutex protecting operations on the map
    gr::thread::mutex d_mutex;

public:
    virtual ~buffer_type_lookup_table() {}

    /*!
     * \brief Returns the singleton object
     *
     */
    static buffer_type_lookup_table* get_instance();

    /*!
     * \brief Insert a new mapping (enum->func_ptr)
     *
     * \details
     *
     * \param e    buffer context enumeration type
     * \param fn   function pointer used to create the associated buffer
     * \return     bool: true if the operation was successful, false otherwise
     */
    // -------------------------------------------------------------------
    // insert:
    //     e:  buffer_context_enum type
    //     fn: function pointer to use as the key in the pair
    // return:
    //     true if the insert operation was successful, false otherwise
    // -----------------------------------------------------------------------
    bool insert(buffer_context_enum e, func_ptr_t fn);



    /*!
     * \brief Lookup and return the function pointer asscociated with 'e'
     *
     * \details
     *
     * \param e    buffer context enumeration type
     * \return     factory function ptr used to create a buffer of type 'e'
     */
    // -----------------------------------------------------------------------
    // lookup:
    //     e:  buffer_context_enum
    //
    // return:
    //     factory method associated with the given enum, or `nullptr` if
    //     there is no existing entry for 'e'
    // -----------------------------------------------------------------------
    func_ptr_t lookup(buffer_context_enum e);

    /*!
     * \brief Removes the entry corresponding to 'e'
     *
     * \details
     *
     * \param e    buffer context enumeration type
     * \return     bool: true if the operation was successful, false otherwise
     */
    // -----------------------------------------------------------------------
    // erase:
    //     e:  buffer_context_enum
    //
    // return:
    //     true if the operation was successful (a mapping existed with the
    //     given 'e', false otherwise
    // -----------------------------------------------------------------------
    bool erase(buffer_context_enum e);
};

} /* namespace gr */

#endif /* GR_RUNTIME_BUFFER_TYPE_LOOKUP_TABLE_H */
