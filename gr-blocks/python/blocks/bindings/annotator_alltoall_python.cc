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
/* BINDTOOL_HEADER_FILE(annotator_alltoall.h)                                        */
/* BINDTOOL_HEADER_FILE_HASH(7f52cd6a04ed3c0db7a7daad5e620f9d)                     */
/***********************************************************************************/

#include <pybind11/complex.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

namespace py = pybind11;

#include <gnuradio/blocks/annotator_alltoall.h>
// pydoc.h is automatically generated in the build directory
#include <annotator_alltoall_pydoc.h>

void bind_annotator_alltoall(py::module& m)
{

    using annotator_alltoall = ::gr::blocks::annotator_alltoall;


    py::class_<annotator_alltoall,
               gr::sync_block,
               gr::block,
               gr::basic_block,
               std::shared_ptr<annotator_alltoall>>(
        m, "annotator_alltoall", D(annotator_alltoall))

        .def(py::init(&annotator_alltoall::make),
             py::arg("when"),
             py::arg("sizeof_stream_item"),
             D(annotator_alltoall, make))


        .def("data", &annotator_alltoall::data, D(annotator_alltoall, data))

        ;
}