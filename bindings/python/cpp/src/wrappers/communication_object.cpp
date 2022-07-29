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
#include <ghex/bindings/python/wrappers/buffer_info.hpp>
#include <ghex/bindings/python/wrappers/communication_handle.hpp>
#include <ghex/bindings/python/wrappers/communication_object.hpp>
#include <ghex/bindings/python/wrappers/specializations.hpp>

namespace py = pybind11;


namespace detail {
    template <typename Arch, typename GridType, typename Transport, typename HaloGenerator, typename Domain, typename Layout>
    using communication_object_wrapper = CommunicationObjectWrapper<Arch, GridType, Transport, HaloGenerator, Domain, Layout>;

    using communication_object_wrapper_specializations = gridtools::meta::transform<gridtools::meta::rename<communication_object_wrapper>::template apply, args>;
}


template <typename Arch, typename GridType, typename Transport, typename HaloGenerator, typename Domain, typename Layout>
struct type_exporter<CommunicationObjectWrapper<Arch, GridType, Transport, HaloGenerator, Domain, Layout>> {
    using buffer_info_wrapper = BufferInfoWrapper<Arch, GridType, Transport, HaloGenerator, Domain, Layout>;
    using communication_handle_wrapper = CommunicationHandleWrapper<Arch, GridType, Transport, HaloGenerator, Domain, Layout>;
    using communication_object_wrapper = CommunicationObjectWrapper<Arch, GridType, Transport, HaloGenerator, Domain, Layout>;
    using communicator_t = typename communication_object_wrapper::communicator_t;

    void operator() (pybind11::module_&, py::class_<communication_object_wrapper> cls) {
        cls.def(py::init([] (communicator_t c) { return communication_object_wrapper(c); }));
        cls.def(
            "exchange",
            [] (communication_object_wrapper& co, buffer_info_wrapper& b) {
                return communication_handle_wrapper(co.co.exchange(b.buffer_info));
            }
        );
    }
};


GHEX_PYBIND11_EXPORT_TYPE(type_exporter, detail::communication_object_wrapper_specializations)