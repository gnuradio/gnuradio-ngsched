/* -*- c++ -*- */
/*
 * Copyright 2004,2009,2010,2013 Free Software Foundation, Inc.
 *
 * This file is part of GNU Radio
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include <gnuradio/block.h>
#include <gnuradio/buffer.h>
#include <gnuradio/buffer_reader_sm.h>
#include <gnuradio/integer_math.h>
#include <gnuradio/math.h>
#include <assert.h>
#include <algorithm>
#include <iostream>
#include <stdexcept>

namespace gr {

buffer_reader_sm::~buffer_reader_sm()
{
}

int buffer_reader_sm::items_available()
{
    int available = 0;
    bool equal = false;
    if (d_buffer->d_write_index == d_read_index)
    {
        // NOTE: d_max_reader_history is always at least one
        if ((nitems_read() - sample_delay()) != d_buffer->nitems_written())
        {
            available = d_buffer->d_bufsize - d_read_index;
            
            if (d_buffer->d_has_history &&
                available <= (int)(d_buffer->d_downstream_lcm_nitems - 1) &&
                (available + d_read_index) == d_buffer->d_bufsize)
            {
                d_read_index = (d_buffer->d_downstream_lcm_nitems - 1) - available;
                available = d_buffer->index_sub(d_buffer->d_write_index, d_read_index);

#ifdef BUFFER_DEBUG
                std::ostringstream msg;
                msg << "[" << d_buffer << ";" << this << "] items_available() RESET";
                GR_LOG_DEBUG(d_logger, msg.str());
#endif
            }
        }
    }
    else
    {
        available = d_buffer->index_sub(d_buffer->d_write_index, d_read_index);
        
        // NOTE: d_max_reader_history is always at least one
        if (d_buffer->d_has_history &&
            available <= (int)(d_buffer->d_downstream_lcm_nitems - 1) &&
            (available + d_read_index) == d_buffer->d_bufsize)
        {
            d_read_index = (d_buffer->d_downstream_lcm_nitems - 1) - available;
            available = d_buffer->index_sub(d_buffer->d_write_index, d_read_index);
            
#ifdef BUFFER_DEBUG
            std::ostringstream msg;
            msg << "[" << d_buffer << ";" << this << "] items_available() RESET";
            GR_LOG_DEBUG(d_logger, msg.str());
#endif
        }
    }

#ifdef BUFFER_DEBUG    
    std::ostringstream msg;
    std::string equalLabel;
    if (equal)
        equalLabel = " [EQUAL] ";
    
    msg << "[" << d_buffer << ";" << this << "] " << equalLabel << "items_available() WR_idx: " 
        << d_buffer->d_write_index << " -- WR items: " << d_buffer->nitems_written()
        << " -- RD_idx: " << d_read_index << " -- RD items: " << nitems_read() 
        << " (-" << d_attr_delay << ") -- available: " << available;
    GR_LOG_DEBUG(d_logger, msg.str());
#endif
    
    return available;
}

buffer_reader_sm::buffer_reader_sm(buffer_sptr buffer, unsigned int read_index, block_sptr link)
    : buffer_reader(buffer, read_index, link)
{
}


} /* namespace gr */