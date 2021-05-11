/* -*- c++ -*- */
/*
 * Copyright 2020 Free Software Foundation, Inc.
 *
 * This file is part of GNU Radio
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 */

#ifndef INCLUDED_GR_RUNTIME_BUFFER_CONTEXT_H
#define INCLUDED_GR_RUNTIME_BUFFER_CONTEXT_H

#include <gnuradio/api.h>
#include <gnuradio/block.h>
#include <gnuradio/buffer.h>
#include <gnuradio/logger.h>
#include <gnuradio/runtime_types.h>

namespace gr {

    enum class buffer_context_t {
        BUFFER_CONTEXT_HOST_TO_DEVICE,
        BUFFER_CONTEXT_DEVICE_TO_HOST,
        BUFFER_CONTEXT_HOST_TO_HOST,
        BUFFER_CONTEXT_DEVICE_TO_DEVICE
    };

    std::ostream& operator<<(std::ostream& os, buffer_type_t buffer_type)
    {
        switch(buffer_type)
        {
        case buffer_type_t::BUFFER_CONTEXT_HOST_TO_DEVICE:
            return os << "BUFFER_CONTEXT_HOST_TO_DEVICE";
        case buffer_type_t::BUFFER_CONTEXT_DEVICE_TO_HOST:
            return os << "BUFFER_CONTEXT_DEVICE_TO_HOST";
        case buffer_type_t::BUFFER_CONTEXT_HOST_TO_HOST:
            return os << "BUFFER_CONTEXT_HOST_TO_HOST";
        case buffer_type_t::BUFFER_CONTEXT_DEVICE_TO_DEVICE:
            return os << "BUFFER_CONTEXT_DEVICE_TO_DEVICE";
        default:
            return os << "unknown buffer type: " << int(buftype);
        }
    }


}
