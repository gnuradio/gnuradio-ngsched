# Copyright 2021 Free Software Foundation, Inc.
#
# This file is part of GNU Radio
#
# SPDX-License-Identifier: GPL-3.0-or-later
#

if(DEFINED __INCLUDED_GR_CUDA_CMAKE)
    return()
endif()
set(__INCLUDED_GR_CUDA_CMAKE TRUE)

########################################################################
# Setup CUDA
########################################################################

find_package(CUDA)