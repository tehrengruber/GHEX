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
#include <ghex/bindings/python/wrappers/specializations.hpp>

namespace py = pybind11;


namespace detail {
    template <typename Arch, typename GridType, typename Transport, typename HaloGenerator, typename Domain, typename Layout>
    using buffer_info_wrapper = BufferInfoWrapper<Arch, GridType, Transport, HaloGenerator, Domain, Layout>;

    using buffer_info_wrapper_specializations = gridtools::meta::transform<gridtools::meta::rename<buffer_info_wrapper>::template apply, args>;
}


template <typename Arch, typename GridType, typename Transport, typename HaloGenerator, typename Domain, typename Layout>
struct type_exporter<BufferInfoWrapper<Arch, GridType, Transport, HaloGenerator, Domain, Layout>> {
    using buffer_info_wrapper = BufferInfoWrapper<Arch, GridType, Transport, HaloGenerator, Domain, Layout>;
    void operator() (pybind11::module_&, py::class_<buffer_info_wrapper>) {};
};


GHEX_PYBIND11_EXPORT_TYPE(type_exporter, detail::buffer_info_wrapper_specializations)