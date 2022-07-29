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

#include <ghex/structured/pattern.hpp>

#include <ghex/bindings/python/wrappers/buffer_info.hpp>
#include <ghex/bindings/python/wrappers/definitions.hpp>


template <typename Arch, typename GridType, typename Transport, typename HaloGenerator, typename Domain, typename Layout>
struct PatternContainerWrapper{
    using definitions = Definitions<Arch, GridType, Transport, HaloGenerator, Domain, Layout>;
    using context_t = typename definitions::context_t;
    using pattern_t = typename definitions::pattern_t;
    using field_t = typename definitions::field_t;
    using buffer_info_t = typename definitions::buffer_info_t;
    using buffer_info_wrapper = BufferInfoWrapper<Arch, GridType, Transport, HaloGenerator, Domain, Layout>;

    pattern_t p;

    PatternContainerWrapper(pattern_t other) : p(other) {};
    PatternContainerWrapper(context_t& context, HaloGenerator& hgen, std::vector<Domain>& d_range) : p(gridtools::ghex::make_pattern<GridType>(context, hgen, d_range)) {};

    buffer_info_wrapper operator()(field_t& field) const {return buffer_info_wrapper(p(field));};

//    PatternContainerWrapper make_pattern(context_t& context, HaloGenerator& hgen, std::vector<Domain>& d_range) {
//        auto other = gridtools::ghex::make_pattern<GridType>(context, hgen, d_range);
//        return PatternContainerWrapper(other);
//    };
};


