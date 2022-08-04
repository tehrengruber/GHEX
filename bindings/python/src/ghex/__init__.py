# -*- coding: utf-8 -*-
#
# GridTools
#
# Copyright (c) 2014-2021, ETH Zurich
# All rights reserved.
#
# Please, refer to the LICENSE file in the root directory.
# SPDX-License-Identifier: BSD-3-Clause
__copyright__ = "Copyright (c) 2014-2021 ETH Zurich"
__license__ = "BSD-3-Clause"

import os
import sys
import warnings

sys.path.append(os.environ.get("GHEX_PY_LIB_PATH", "/home/tille/Development/GHEX/build"))

from ghex.utils.cpp_wrapper_utils import unwrap
import ghex_py_bindings as _ghex


def _may_use_mpi4py():
    try:
        import mpi4py

        return True
    except:
        return False


def _validate_library_version():
    """check mpi library version string of mpi4py and bindings match"""
    if not _may_use_mpi4py():
        return
    import mpi4py.MPI

    ghex_mpi_lib_ver = _ghex.utils.mpi_library_version()
    mpi4py_lib_ver = mpi4py.MPI.Get_library_version()
    # fix erroneous nullbyte at the end
    if mpi4py_lib_ver[-1] == "\x00":
        mpi4py_lib_ver = mpi4py_lib_ver[:-1]
    if ghex_mpi_lib_ver != mpi4py_lib_ver:
        warnings.warn(
            f"GHEX and mpi4py were compiled using different mpi versions.\n"
            f" GHEX:   {ghex_mpi_lib_ver}\n"
            f" mpi4py: {mpi4py_lib_ver}."
        )


_validate_library_version()


from .structured.regular import *

from . import tl
