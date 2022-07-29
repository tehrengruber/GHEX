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

#include <ghex/bindings/python/wrappers/communication_object.hpp>


template <typename Arch, typename GridType, typename Transport, typename HaloGenerator, typename Domain, typename Layout>
struct CommunicationHandleWrapper{
    using communication_object_wrapper = CommunicationObjectWrapper<Arch, GridType, Transport, HaloGenerator, Domain, Layout>;
    using communication_object_t = typename communication_object_wrapper::communication_object_t;
    using handle_t = typename communication_object_t::handle_type;

    handle_t handle;

    CommunicationHandleWrapper(handle_t&& other) : handle(std::move(other)) {};

    void wait() { handle.wait(); };
};
