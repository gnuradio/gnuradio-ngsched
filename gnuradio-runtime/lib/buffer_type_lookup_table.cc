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

namespace gr {

// NB: This buffer_type_t doesn't match the pre-existing buffer_type_t
//     since this type is an enum class and the pre-existing type
//     is a plain old object
//
//     Do we want to keep the enum class, keep the pre-existing type,
//     or something else?
std::ostream& operator<<(std::ostream& os, buffer_type_t buffer_type)
{
    switch(buffer_type)
    {
    case buffer_type_t::BUFFER_TYPE_HOST:
        return os << "BUFFER_TYPE_HOST";
    case buffer_type_t::BUFFER_TYPE_CUDA:
        return os << "BUFFER_TYPE_CUDA";
    case buffer_type_t::BUFFER_TYPE_HIP:
        return os << "BUFFER_TYPE_HIP";
    default:
        return os << "BUFFER TYPE UNKNOWN: " << int(buftype)
    }
}

buffer_type_lookup_table* buffer_type_lookup_table::get_instance()
{
    static buffer_type_lookup_table instance;
    return &instance;
}

bool buffer_type_lookup_table::insert(buffer_type_t buffer_type,
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
        cout << "**** dupe: " << e << endl;
        return false;
    }
}

func_ptr_t buffer_type_lookup_table::lookup(buffer_type_t buffer_type)
{
    unique_lock<std::mutex> lck(d_mutex);

    cout << "**** lokking up: " << buffer_type << endl;

    if (d_map.find(buffer_type) != d_map.end()) {
        // we found it
        return d_map[buffer_type];
    }
    else {
        cout << "**** lookup failed" << endl;
        return nullptr;
    }
}


bool buffer_type_lookup_table::erase(buffer_type_t buffer_type)
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

} /* namespace gr */
