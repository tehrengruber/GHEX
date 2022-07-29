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

#include <ghex/arch_traits.hpp>
#include <ghex/communication_object_2.hpp>
#include <ghex/transport_layer/context.hpp>
#include <ghex/transport_layer/mpi/context.hpp>

#include <ghex/structured/pattern.hpp>
#include <ghex/structured/regular/domain_descriptor.hpp>
#include <ghex/structured/regular/halo_generator.hpp>
#include <ghex/structured/regular/field_descriptor.hpp>


template <typename Arch, typename GridType, typename Transport, typename HaloGenerator, typename Domain, typename Layout>
struct Definitions{
    using grid_t = typename GridType::template type<Domain>;
    using context_t = typename gridtools::ghex::tl::context_factory<Transport>::context_type;
    using domain_id_t = typename Domain::domain_id_type;
    using domain_descriptor_t = typename gridtools::ghex::structured::regular::domain_descriptor<domain_id_t, std::integral_constant<int, 3>>;
    using pattern_t = decltype(gridtools::ghex::make_pattern<GridType>(std::declval<context_t&>(), std::declval<HaloGenerator&>(), std::declval<std::vector<Domain>&>()));
    using array_t = std::array<int, 3>;
    using field_t = decltype(gridtools::ghex::wrap_field<Arch, Layout>(std::declval<const domain_descriptor_t&>(), std::declval<double*>(), std::declval<const array_t&>(), std::declval<const array_t&>()));
    using buffer_info_t = decltype(std::declval<pattern_t>()(std::declval<field_t&>()));
};