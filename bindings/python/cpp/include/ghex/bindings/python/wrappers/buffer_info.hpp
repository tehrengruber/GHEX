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
#pragma once

#include <pybind11/pybind11.h>

#include <ghex/bindings/python/utils/type_exporter.hpp>
#include <ghex/bindings/python/wrappers/definitions.hpp>

namespace py = pybind11;


template <typename Arch, typename GridType, typename Transport, typename HaloGenerator, typename Domain, typename Layout>
struct buffer_info_wrapper{
    using defs = definitions<Arch, GridType, Transport, HaloGenerator, Domain, Layout>;
    using buffer_info_t = typename defs::buffer_info_t;

    buffer_info_t buffer_info;

    buffer_info_wrapper(buffer_info_t&& other) : buffer_info(std::move(other)) {};
};


template <typename Arch, typename GridType, typename Transport, typename HaloGenerator, typename Domain, typename Layout>
struct type_exporter<buffer_info_wrapper<Arch, GridType, Transport, HaloGenerator, Domain, Layout>> {
    using buffer_info_w = buffer_info_wrapper<Arch, GridType, Transport, HaloGenerator, Domain, Layout>;
    void operator() (pybind11::module_&, py::class_<buffer_info_w>) {};
};