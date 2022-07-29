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

#include <pybind11/pybind11.h>

#include <ghex/bindings/python/utils/type_exporter.hpp>
#include <ghex/bindings/python/wrappers/communication_handle.hpp>
#include <ghex/bindings/python/wrappers/specializations.hpp>

namespace py = pybind11;


namespace detail {
    template <typename Arch, typename GridType, typename Transport, typename HaloGenerator, typename Domain, typename Layout>
    using communication_handle_wrapper = CommunicationHandleWrapper<Arch, GridType, Transport, HaloGenerator, Domain, Layout>;

    using communication_handle_wrapper_specializations = gridtools::meta::transform<gridtools::meta::rename<communication_handle_wrapper>::template apply, args>;
}


template <typename Arch, typename GridType, typename Transport, typename HaloGenerator, typename Domain, typename Layout>
struct type_exporter<CommunicationHandleWrapper<Arch, GridType, Transport, HaloGenerator, Domain, Layout>> {
    using communication_handle_wrapper = CommunicationHandleWrapper<Arch, GridType, Transport, HaloGenerator, Domain, Layout>;

    void operator() (pybind11::module_&, py::class_<communication_handle_wrapper> cls) {
        cls.def("wait", &communication_handle_wrapper::wait);
    }
};


GHEX_PYBIND11_EXPORT_TYPE(type_exporter, detail::communication_handle_wrapper_specializations)