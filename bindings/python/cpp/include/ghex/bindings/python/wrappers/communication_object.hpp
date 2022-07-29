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

//#include <utility>
//#include <vector>
//
//#include <ghex/arch_traits.hpp>
//#include <ghex/communication_object_2.hpp>
//#include <ghex/transport_layer/context.hpp>
//#include <ghex/transport_layer/mpi/context.hpp>
//
//#include <ghex/structured/pattern.hpp>
//#include <ghex/structured/regular/domain_descriptor.hpp>
//#include <ghex/structured/regular/halo_generator.hpp>
//#include <ghex/structured/regular/field_descriptor.hpp>

#include <ghex/bindings/python/wrappers/definitions.hpp>


template <typename Arch, typename GridType, typename Transport, typename HaloGenerator, typename Domain, typename Layout>
struct CommunicationObjectWrapper{
    using definitions = Definitions<Arch, GridType, Transport, HaloGenerator, Domain, Layout>;
    using communicator_t = typename definitions::context_t::communicator_type;
    using grid_t = typename definitions::grid_t;
    using domain_id_t = typename definitions::domain_id_t;
    using communication_object_t = gridtools::ghex::communication_object<communicator_t, grid_t, domain_id_t>;

    communication_object_t co;

    CommunicationObjectWrapper(communicator_t comm) : co(comm) {};
};


