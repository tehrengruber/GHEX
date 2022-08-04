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

#include <vector>

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include <ghex/structured/pattern.hpp>

#include <ghex/bindings/python/utils/type_exporter.hpp>
#include <ghex/bindings/python/wrappers/buffer_info.hpp>
#include <ghex/bindings/python/wrappers/definitions.hpp>


template <typename Arch, typename GridType, typename Transport, typename HaloGenerator, typename Domain, typename Layout>
struct pattern_container_wrapper{
    using defs = definitions<Arch, GridType, Transport, HaloGenerator, Domain, Layout>;
    using context_t = typename defs::context_t;
    using pattern_t = typename defs::pattern_t;
    using field_t = typename defs::field_t;
    using buffer_info_t = typename defs::buffer_info_t;
    using buffer_info_w = buffer_info_wrapper<Arch, GridType, Transport, HaloGenerator, Domain, Layout>;

    pattern_t p;

    pattern_container_wrapper(pattern_t other) : p(other) {};
    pattern_container_wrapper(context_t& context, HaloGenerator& hgen, std::vector<Domain>& d_range) :
        p(gridtools::ghex::make_pattern<GridType>(context, hgen, d_range)) {};

    buffer_info_w operator()(field_t& field) const {return buffer_info_w(p(field));};
};


template <typename Arch, typename GridType, typename Transport, typename HaloGenerator, typename Domain, typename Layout>
struct type_exporter<pattern_container_wrapper<Arch, GridType, Transport, HaloGenerator, Domain, Layout>> {
    using pattern_container_w = pattern_container_wrapper<Arch, GridType, Transport, HaloGenerator, Domain, Layout>;
    using context_t = typename pattern_container_w::defs::context_t;
    using buffer_info_t = typename pattern_container_w::defs::buffer_info_t;

    void operator() (pybind11::module_&, py::class_<pattern_container_w> cls) {
        cls.def(py::init([] (context_t& context, HaloGenerator& hgen, std::vector<Domain>& d_range) {
                return pattern_container_w(context, hgen, d_range);
            }))
           .def("__call__", &pattern_container_w::operator());
    }
};


