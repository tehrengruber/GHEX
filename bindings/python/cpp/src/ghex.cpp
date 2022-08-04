/*
 * GridTools
 *
 * Copyright (c) 2014-2021, ETH Zurich
 * All rights reserved.
 *
 * Please, refer to the LICENSE file in the root directory.
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */
#include <sstream>
#include <utility>
#include <vector>

#include <mpi.h>

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include <ghex/structured/pattern.hpp>
#include <ghex/transport_layer/context.hpp>
#include <ghex/transport_layer/mpi/context.hpp>

#include <ghex/bindings/python/binding_registry.hpp>
#include <ghex/bindings/python/utils/mpi_comm_shim.hpp>

namespace py = pybind11;


PYBIND11_MODULE(ghex_py_bindings, m) {
    gridtools::ghex::bindings::python::BindingRegistry::get_instance().set_initialized(m);

    m.doc() = "pybind11 ghex bindings"; // optional module docstring

    m.def_submodule("utils")
        .def("hash_str", [] (const std::string& val) { return std::hash<std::string>()(val); })
        .def("mpi_library_version", [] () {
            int resultlen;
            char version[MPI_MAX_LIBRARY_VERSION_STRING];
            MPI_Get_library_version(version, &resultlen);
            return std::string(version);
        });

    pybind11::class_<mpi_comm_shim> mpi_comm(m, "mpi_comm");
    mpi_comm.def(pybind11::init<>());

    py::class_<gridtools::ghex::coordinate<std::array<int, 3>>>(m, "Coordinate3d");
}