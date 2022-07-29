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

#include <ghex/bindings/python/wrappers/definitions.hpp>


template <typename Arch, typename GridType, typename Transport, typename HaloGenerator, typename Domain, typename Layout>
struct BufferInfoWrapper{
    using definitions = Definitions<Arch, GridType, Transport, HaloGenerator, Domain, Layout>;
    using buffer_info_t = typename definitions::buffer_info_t;

    buffer_info_t buffer_info;

    BufferInfoWrapper(buffer_info_t&& other) : buffer_info(std::move(other)) {};
};
