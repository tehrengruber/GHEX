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

#include <ghex/bindings/python/type_list.hpp>

namespace ghex4py = gridtools::ghex::bindings::python;


namespace detail {

    using args_cpu = gridtools::meta::cartesian_product<
        gridtools::meta::list<gridtools::ghex::cpu>,
        gridtools::meta::list<gridtools::ghex::structured::grid>,
        gridtools::meta::list<ghex4py::type_list::transport>,
        gridtools::meta::list<ghex4py::type_list::halo_generator_type>,
        gridtools::meta::list<ghex4py::type_list::domain_descriptor_type>,
        gridtools::meta::list<
            ::gridtools::layout_map<0, 1, 2>,
            ::gridtools::layout_map<2, 1, 0>,
            ::gridtools::layout_map<1, 2, 0>,
            ::gridtools::layout_map<2, 0, 1>
        >
    >;

#ifdef __CUDACC__
    using args_gpu = gridtools::meta::cartesian_product<
        gridtools::meta::list<gridtools::ghex::gpu>,
        gridtools::meta::list<gridtools::ghex::structured::grid>,
        gridtools::meta::list<ghex4py::type_list::transport>,
        gridtools::meta::list<ghex4py::type_list::halo_generator_type>,
        gridtools::meta::list<ghex4py::type_list::domain_descriptor_type>,
        gridtools::meta::list<
            ::gridtools::layout_map<0, 1, 2>,
            ::gridtools::layout_map<2, 1, 0>,
            ::gridtools::layout_map<1, 2, 0>,
            ::gridtools::layout_map<2, 0, 1>
        >
    >;
#endif

}