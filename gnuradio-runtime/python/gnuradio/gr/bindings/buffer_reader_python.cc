/*
 * Copyright 2020 Free Software Foundation, Inc.
 *
 * This file is part of GNU Radio
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 */

/***********************************************************************************/
/* This file is automatically generated using bindtool and can be manually edited  */
/* The following lines can be configured to regenerate this file during cmake      */
/* If manual edits are made, the following tags should be modified accordingly.    */
/* BINDTOOL_GEN_AUTOMATIC(0)                                                       */
/* BINDTOOL_USE_PYGCCXML(0)                                                        */
/* BINDTOOL_HEADER_FILE(buffer_reader.h)                                           */
/* BINDTOOL_HEADER_FILE_HASH(b54466f5feabd699d431d196806b37ce)                     */
/***********************************************************************************/

#include <pybind11/complex.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

namespace py = pybind11;

#include <gnuradio/block.h>
#include <gnuradio/buffer_reader.h>
// pydoc.h is automatically generated in the build directory
#include <buffer_pydoc.h>

void bind_buffer_reader(py::module& m)
{

    using buffer_reader = ::gr::buffer_reader;

    py::class_<buffer_reader, std::shared_ptr<buffer_reader>>(
        m, "buffer_reader", D(buffer_reader))

        .def(py::init<gr::buffer_reader const&>(),
             py::arg("arg0"),
             D(buffer_reader, buffer_reader))


        .def("declare_sample_delay",
             &buffer_reader::declare_sample_delay,
             py::arg("delay"),
             D(buffer_reader, declare_sample_delay))


        .def("sample_delay", &buffer_reader::sample_delay, D(buffer_reader, sample_delay))


        .def("items_available",
             &buffer_reader::items_available,
             D(buffer_reader, items_available))


        .def("buffer", &buffer_reader::buffer, D(buffer_reader, buffer))


        .def("max_possible_items_available",
             &buffer_reader::max_possible_items_available,
             D(buffer_reader, max_possible_items_available))


        .def("read_pointer", &buffer_reader::read_pointer, D(buffer_reader, read_pointer))


        .def("update_read_pointer",
             &buffer_reader::update_read_pointer,
             py::arg("nitems"),
             D(buffer_reader, update_read_pointer))


        .def("set_done",
             &buffer_reader::set_done,
             py::arg("done"),
             D(buffer_reader, set_done))


        .def("done", &buffer_reader::done, D(buffer_reader, done))


        .def("mutex", &buffer_reader::mutex, D(buffer_reader, mutex))


        .def("nitems_read", &buffer_reader::nitems_read, D(buffer_reader, nitems_read))


        .def("reset_nitem_counter",
             &buffer_reader::reset_nitem_counter,
             D(buffer_reader, reset_nitem_counter))


        .def("get_sizeof_item",
             &buffer_reader::get_sizeof_item,
             D(buffer_reader, get_sizeof_item))


        .def("link", &buffer_reader::link, D(buffer_reader, link))


        .def("get_tags_in_range",
             &buffer_reader::get_tags_in_range,
             py::arg("v"),
             py::arg("abs_start"),
             py::arg("abs_end"),
             py::arg("id"),
             D(buffer_reader, get_tags_in_range))

        ;


    m.def("buffer_add_reader",
          &::gr::buffer_add_reader,
          py::arg("buf"),
          py::arg("nzero_preload"),
          py::arg("link") = gr::block_sptr(),
          py::arg("delay") = 0,
          D(buffer_add_reader));


    m.def("buffer_reader_ncurrently_allocated",
          &::gr::buffer_reader_ncurrently_allocated,
          D(buffer_reader_ncurrently_allocated));
}
