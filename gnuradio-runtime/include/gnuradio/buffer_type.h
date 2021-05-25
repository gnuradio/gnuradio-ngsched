/* -*- c++ -*- */
/*
 * Copyright 2020 Free Software Foundation, Inc.
 *
 * This file is part of GNU Radio
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 */

#ifndef INCLUDED_GR_RUNTIME_CUSTOM_BUFFER_TYPE_H
#define INCLUDED_GR_RUNTIME_CUSTOM_BUFFER_TYPE_H

#include <gnuradio/api.h>
#include <gnuradio/runtime_types.h>

#include <cstdint>
#include <mutex>
#include <string>

namespace gr {


typedef void (*factory_func_ptr)(int);

GR_RUNTIME_API void foobar(int arg);



class GR_RUNTIME_API buffer_type_base
{
public:
    virtual ~buffer_type_base(){};

    // Do not allow copying or assignment
    buffer_type_base(buffer_type_base const&) = delete;
    void operator=(buffer_type_base const&) = delete;

    // Allow equality and inequality comparison
    bool operator==(const buffer_type_base& other) const
    {
        return d_value == other.d_value;
    }

    bool operator!=(const buffer_type_base& other) const
    {
        return d_value != other.d_value;
    }

    // Do not allow other comparison (just in case)
    bool operator<(const buffer_type_base& other) = delete;
    bool operator>(const buffer_type_base& other) = delete;
    bool operator<=(const buffer_type_base& other) = delete;
    bool operator>=(const buffer_type_base& other) = delete;

    const std::string& name() const { return d_name; }

    inline buffer_sptr make_buffer(int nitems,
                                   size_t sizeof_item,
                                   uint64_t downstream_lcm_nitems,
                                   block_sptr link,
                                   block_sptr buf_owner) const
    {
        // Delegate call to factory function
        return d_factory(nitems, sizeof_item, downstream_lcm_nitems, link, buf_owner);
    }

protected:
    static uint32_t s_nextId;
    static std::mutex s_mutex;

    uint32_t d_value;
    std::string d_name;
    factory_func_ptr d_factory;

    // Protected constructor
    buffer_type_base(const char* name, factory_func_ptr factory_func)
        : d_name(name), d_factory(factory_func)
    {
        std::lock_guard<std::mutex> lock(s_mutex);
        d_value = s_nextId++;
    }
};

typedef const buffer_type_base buffer_type_t;


#define MAKE_CUSTOM_BUFFER_TYPE(CLASSNAME, FACTORY_FUNC_PTR)                      \
    class GR_RUNTIME_API buftype_##CLASSNAME : public buffer_type_base            \
    {                                                                             \
    public:                                                                       \
        static buffer_type get()                                                  \
        {                                                                         \
            static buftype_##CLASSNAME instance;                                  \
            return instance;                                                      \
        }                                                                         \
                                                                                  \
    private:                                                                      \
        buftype_##CLASSNAME() : buffer_type_base(#CLASSNAME, FACTORY_FUNC_PTR) {} \
    };

} // namespace gr

#endif /* INCLUDED_GR_RUNTIME_CUSTOM_BUFFER_TYPE_H */
