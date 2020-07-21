/* -*- c++ -*- */
/*
 * Copyright 2004,2009-2011,2013 Free Software Foundation, Inc.
 *
 * This file is part of GNU Radio
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 */

#ifndef INCLUDED_GR_RUNTIME_BUFFER_READER_SM_H
#define INCLUDED_GR_RUNTIME_BUFFER_READER_SM_H

#include <gnuradio/api.h>
#include <gnuradio/logger.h>
#include <gnuradio/buffer.h>
#include <gnuradio/buffer_reader.h>
#include <gnuradio/runtime_types.h>
#include <gnuradio/tags.h>
#include <gnuradio/thread/thread.h>
#include <boost/weak_ptr.hpp>
#include <map>
#include <memory>

namespace gr {
    
class GR_RUNTIME_API buffer_reader_sm : public buffer_reader
{
public:
    
    ~buffer_reader_sm();
    
    /*!
     * \brief Return number of items available for reading.
     */
    virtual int items_available();  // const
    
private:
    friend class buffer;
    friend class buffer_double_mapped;
    friend class buffer_single_mapped;
    
    friend GR_RUNTIME_API buffer_reader_sptr buffer_add_reader(buffer_sptr buf,
                                                               int nzero_preload,
                                                               block_sptr link,
                                                               int delay);
    
    //! constructor is private.  Use gr::buffer::add_reader to create instances
    buffer_reader_sm(buffer_sptr buffer, unsigned int read_index, block_sptr link);
};

}

#endif /* INCLUDED_GR_RUNTIME_BUFFER_READER_SM_H */