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

#include <ghex/bindings/python/utils/type_exporter.hpp>
#include <ghex/bindings/python/wrappers/communication.hpp>
#include <ghex/bindings/python/wrappers/specializations.hpp>


namespace detail {

    template <typename Arch, typename GridType, typename Transport, typename HaloGenerator, typename Domain, typename Layout>
    using communication_object_w = communication_object_wrapper<Arch, GridType, Transport, HaloGenerator, Domain, Layout>;

    using specializations = gridtools::meta::transform<gridtools::meta::rename<communication_object_w>::template apply, args_gpu>;

}


GHEX_PYBIND11_EXPORT_TYPE(type_exporter, detail::specializations)