# -*- coding: utf-8 -*-
#
# GridTools
#
# Copyright (c) 2014-2021, ETH Zurich
# All rights reserved.
#
# Please, refer to the LICENSE file in the root directory.
# SPDX-License-Identifier: BSD-3-Clause

from __future__ import annotations
from typing import TYPE_CHECKING

from ghex.utils.cpp_wrapper_utils import CppWrapper

if TYPE_CHECKING:
    from mpi4py.MPI import Comm


class Context(CppWrapper):
    def __init__(self, mpi_comm: Comm) -> None:
        super(Context, self).__init__(
            "gridtools::ghex::tl::context<gridtools::ghex::tl::mpi::transport_context>", mpi_comm
        )
