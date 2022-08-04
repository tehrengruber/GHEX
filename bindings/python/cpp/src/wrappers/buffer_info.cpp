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

#include <ghex/bindings/python/wrappers/buffer_info.hpp>
#include <ghex/bindings/python/wrappers/specializations.hpp>


namespace detail {

    template <typename Arch, typename GridType, typename Transport, typename HaloGenerator, typename Domain, typename Layout>
    using buffer_info_w = buffer_info_wrapper<Arch, GridType, Transport, HaloGenerator, Domain, Layout>;

    using specializations = gridtools::meta::transform<gridtools::meta::rename<buffer_info_w>::template apply, args_cpu>;

}


GHEX_PYBIND11_EXPORT_TYPE(type_exporter, detail::specializations)