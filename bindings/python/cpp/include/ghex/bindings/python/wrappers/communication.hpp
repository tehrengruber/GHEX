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
#include <ghex/bindings/python/wrappers/buffer_info.hpp>
#include <ghex/bindings/python/wrappers/definitions.hpp>

namespace py = pybind11;


template <typename Arch, typename GridType, typename Transport, typename HaloGenerator, typename Domain, typename Layout>
struct communication_object_wrapper{
    using defs = definitions<Arch, GridType, Transport, HaloGenerator, Domain, Layout>;
    using communicator_t = typename defs::context_t::communicator_type;
    using grid_t = typename defs::grid_t;
    using domain_id_t = typename defs::domain_id_t;
    using communication_object_t = gridtools::ghex::communication_object<communicator_t, grid_t, domain_id_t>;

    communication_object_t co;

    communication_object_wrapper(communicator_t comm) : co(comm) {};
};


template <typename Arch, typename GridType, typename Transport, typename HaloGenerator, typename Domain, typename Layout>
struct communication_handle_wrapper{
    using communication_object_w = communication_object_wrapper<Arch, GridType, Transport, HaloGenerator, Domain, Layout>;
    using communication_object_t = typename communication_object_w::communication_object_t;
    using handle_t = typename communication_object_t::handle_type;

    handle_t handle;

    communication_handle_wrapper(handle_t&& other) : handle(std::move(other)) {};

    void wait() { handle.wait(); };
};


template <typename Arch, typename GridType, typename Transport, typename HaloGenerator, typename Domain, typename Layout>
struct type_exporter<communication_object_wrapper<Arch, GridType, Transport, HaloGenerator, Domain, Layout>> {
    using buffer_info_w = buffer_info_wrapper<Arch, GridType, Transport, HaloGenerator, Domain, Layout>;
    using communication_handle_w = communication_handle_wrapper<Arch, GridType, Transport, HaloGenerator, Domain, Layout>;
    using communication_object_w = communication_object_wrapper<Arch, GridType, Transport, HaloGenerator, Domain, Layout>;
    using communicator_t = typename communication_object_w::communicator_t;

    void operator() (pybind11::module_&, py::class_<communication_object_w> cls) {
        cls.def(py::init([] (communicator_t c) { return communication_object_w(c); }));
        cls.def(
            "exchange",
            [] (communication_object_w& co, buffer_info_w& b) {
                return communication_handle_w(co.co.exchange(b.buffer_info));
            }
        );
    }
};


template <typename Arch, typename GridType, typename Transport, typename HaloGenerator, typename Domain, typename Layout>
struct type_exporter<communication_handle_wrapper<Arch, GridType, Transport, HaloGenerator, Domain, Layout>> {
    using communication_handle_w = communication_handle_wrapper<Arch, GridType, Transport, HaloGenerator, Domain, Layout>;

    void operator() (pybind11::module_&, py::class_<communication_handle_w> cls) {
        cls.def("wait", &communication_handle_w::wait);
    }
};
