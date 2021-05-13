/* -*- c++ -*- */
/*
 * Copyright 2012 Free Software Foundation, Inc.
 *
 * This file is part of GNU Radio
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 */

#include <gnuradio/buffer_type_lookup_table.h>
#include <gnuradio/buffer_type.h>
#include <gnuradio/cuda_buffer.h>
#include <gnuradio/hip_buffer.h>
#include <gnuradio/host_buffer.h>


namespace gr {

// initialize the static map
// NB:  When adding new custom buffer types you must update
//      the following map-literal expression to include a
//      mapping for your new type
// buffer_type_lookup_table::buffer_type_to_fn_ptr_map
// buffer_type_lookup_table::d_map = {
//     {buftype_DEFAULT_CUDA::get(), &cuda_buffer::make_cuda_buffer},
//     {buftype_DEFAULT_HIP::get(),  &hip_buffer::make_hip_buffer},
//     {buftype_DEFAULT_HOST::get(), &host_buffer::make_host_buffer},
// };

 buffer_type_lookup_table::buffer_type_lookup_table()
{
    d_map[std::ref(buftype_DEFAULT_CUDA::get())] = &cuda_buffer::make_cuda_buffer;
}

#if 0
// NB: This buffer_type_t doesn't match the pre-existing buffer_type_t
//     since this type is an enum class and the pre-existing type
//     is a plain old object
//
//     Do we want to keep the enum class, keep the pre-existing type,
//     or something else?
std::ostream& operator<<(std::ostream& os, buffer_type buffer_type)
{
    switch(buffer_type)
    {
    case buftype_DEFAULT_CUDA:
        return os << "buftype_DEFAULT_CUDA";
    case buftype_DEFAULT_HIP:
        return os << "buftype_DEFAULT_HIP";
    case buftype_DEFAULT_HOST:
        return os << "buftype_DEFAULT_HOST";
    default:
        return os << "BUFFER TYPE UNKNOWN: " << buffer_type;
    }
}
#endif

#if 0
buffer_type_lookup_table& buffer_type_lookup_table::get_instance()
{
    static buffer_type_lookup_table instance;
    return instance;
}
#endif

//bool buffer_type_lookup_table::insert(buffer_type_t buffer_type,
bool buffer_type_lookup_table::insert(buffer_type_t& buffer_type,
                                      func_ptr_t(*fn)(int))
{
    unique_lock<std::mutex> lck(d_mutex);

    if (d_map.find(buffer_type) == d_map.end()) {
        // we found it
        d_map[buffer_type] = fn;
        cout << "**** inserted: " << e << endl;
        return true;
    }
    else {
        cout << "**** duplicate keys are not allowed" << endl;
        cout << "**** dupe: " << buffer_type << endl;
        return false;
    }
}

func_ptr_t buffer_type_lookup_table::lookup(buffer_type_base* buffer_type)
{
    unique_lock<std::mutex> lck(d_mutex);

    if (d_map.find(buffer_type) != d_map.end()) {
        // we found it
        return d_map[buffer_type];
    }
    else {
        return nullptr;
    }
}

bool buffer_type_lookup_table::erase(buffer_type_base* buffer_type)
{
    unique_lock<std::mutex> lck(d_mutex);

    if (d_map.find(buffer_type) != d_map.end()) {
        // we found it, now delete it
        cout << "**** deleting entry: " << buffer_type << endl;
        d_map.erase(buffer_type);
        return true;
    }
    else {
        cout << "**** delete failed (item not found)" << endl;
        cout << "**** " << buffer_type << endl;
        return false;
    }
}
#endif

} /* namespace gr */
