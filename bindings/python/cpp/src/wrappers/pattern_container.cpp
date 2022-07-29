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
#include <pybind11/stl.h>

#include <ghex/bindings/python/utils/type_exporter.hpp>

#include <ghex/bindings/python/wrappers/pattern_container.hpp>
#include <ghex/bindings/python/wrappers/specializations.hpp>

namespace py = pybind11;


namespace detail {
    template <typename Arch, typename GridType, typename Transport, typename HaloGenerator, typename Domain, typename Layout>
    using pattern_container_wrapper = PatternContainerWrapper<Arch, GridType, Transport, HaloGenerator, Domain, Layout>;

    using pattern_container_wrapper_specializations = gridtools::meta::transform<gridtools::meta::rename<pattern_container_wrapper>::template apply, args>;
}


template <typename Arch, typename GridType, typename Transport, typename HaloGenerator, typename Domain, typename Layout>
struct type_exporter<PatternContainerWrapper<Arch, GridType, Transport, HaloGenerator, Domain, Layout>> {
    using pattern_container_wrapper = PatternContainerWrapper<Arch, GridType, Transport, HaloGenerator, Domain, Layout>;
    using context_t = typename pattern_container_wrapper::definitions::context_t;
    using buffer_info_t = typename pattern_container_wrapper::definitions::buffer_info_t;

    void operator() (pybind11::module_&, py::class_<pattern_container_wrapper> cls) {
        cls.def(py::init([] (context_t& context, HaloGenerator& hgen, std::vector<Domain>& d_range) {
            return pattern_container_wrapper(context, hgen, d_range);
        }));
        cls.def("__call__", &pattern_container_wrapper::operator());
    }
};


GHEX_PYBIND11_EXPORT_TYPE(type_exporter, detail::pattern_container_wrapper_specializations)