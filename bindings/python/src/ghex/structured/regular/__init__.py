# -*- coding: utf-8 -*-
#
# GridTools
#
# Copyright (c) 2014-2021, ETH Zurich
# All rights reserved.
#
# Please, refer to the LICENSE file in the root directory.
# SPDX-License-Identifier: BSD-3-Clause

from __future__ import annotations
from typing import TYPE_CHECKING

import ghex_py_bindings as _ghex
from ghex.utils.cpp_wrapper_utils import CppWrapper, dtype_to_cpp, unwrap
from ghex.utils.index_space import ProductSet, union

if TYPE_CHECKING:
    import numpy as np
    from typing import Literal, Union

    from ghex.tl import Context
    from ghex.utils.index_space import CartesianSet


# todo: call HaloContainer?
class HaloContainer:
    local: CartesianSet
    global_: CartesianSet

    def __init__(self, local: CartesianSet, global_: CartesianSet) -> None:
        self.local = local
        self.global_ = global_


class DomainDescriptor(CppWrapper):
    def __init__(self, id_: int, sub_domain_indices: ProductSet) -> None:
        super(DomainDescriptor, self).__init__(
            ("gridtools::ghex::structured::regular::domain_descriptor", "int", 3),
            id_,
            sub_domain_indices[0, 0, 0],
            sub_domain_indices[-1, -1, -1],
        )


class HaloGenerator(CppWrapper):
    def __init__(
        self,
        glob_domain_indices: ProductSet,
        halos: tuple[Union[int, tuple[int, int]], ...],
        periodicity: tuple[bool, ...],
    ) -> None:
        assert glob_domain_indices.dim == len(halos)
        assert glob_domain_indices.dim == len(periodicity)

        # canonanicalize integer halos, e.g. turn (h0, (h1, h2), h3) into ((h0, h0), (h1, h2), ...)
        halos = ((halo, halo) if isinstance(halo, int) else halo for halo in halos)
        flattened_halos = tuple(h for halo in halos for h in halo)

        super(HaloGenerator, self).__init__(
            ("gridtools::ghex::structured::regular::halo_generator", "int", 3),
            glob_domain_indices[0, 0, 0],
            glob_domain_indices[-1, -1, -1],
            flattened_halos,
            periodicity,
        )

    def __call__(self, domain: DomainDescriptor) -> HaloContainer:
        result = self.__wrapped_call__("__call__", domain)

        local = union(
            *(
                ProductSet.from_coords(tuple(box2.local.first), tuple(box2.local.last))
                for box2 in result
            )
        )
        global_ = union(
            *(
                ProductSet.from_coords(tuple(box2.global_.first), tuple(box2.global_.last))
                for box2 in result
            )
        )
        return HaloContainer(local, global_)


# todo: try importing gt4py to see if it's there, avoiding the dependency


def _layout_order(field: np.ndarray) -> tuple[int, ...]:
    ordered_strides = list(reversed(sorted(field.strides)))
    layout_map = tuple(ordered_strides.index(stride) for stride in field.strides)
    return layout_map


class FieldDescriptor(CppWrapper):
    def __init__(
        self,
        device: Literal["cpu", "gpu"],
        domain_desc: DomainDescriptor,
        field: np.ndarray,
        offsets: tuple[int, ...],
        extents: tuple[int, ...],
    ) -> None:
        type_spec = (
            "gridtools::ghex::structured::regular::field_descriptor",
            dtype_to_cpp(field.dtype),
            f"gridtools::ghex::{device}",
            domain_desc.__cpp_type__,
            f"gridtools::layout_map<{', '.join(map(str, _layout_order(field)))}> ",
        )
        super(FieldDescriptor, self).__init__(type_spec, domain_desc, field, offsets, extents)


def wrap_field(*args):
    return FieldDescriptor(*args)


class CommunicationObject(CppWrapper):
    def __init__(
        self, device: Literal["cpu", "gpu"], layout_map: tuple[int, ...], communicator
    ) -> None:
        type_spec = (
            "communication_object_wrapper",
            f"gridtools::ghex::{device}",
            "gridtools::ghex::structured::grid",
            "gridtools::ghex::tl::mpi_tag",
            "gridtools::ghex::structured::regular::halo_generator<int, std::integral_constant<int, 3> >",
            "gridtools::ghex::structured::regular::domain_descriptor<int, std::integral_constant<int, 3> >",
            f"gridtools::layout_map<{', '.join(map(str, layout_map))}> ",
        )
        super(CommunicationObject, self).__init__(type_spec, communicator)


class PatternContainer(CppWrapper):
    def __init__(
        self,
        device: Literal["cpu", "gpu"],
        layout_map: tuple[int, ...],
        context: Context,
        halo_generator: HaloGenerator,
        domain_range: tuple[DomainDescriptor, ...],
    ) -> None:
        type_spec = (
            "pattern_container_wrapper",
            f"gridtools::ghex::{device}",
            "gridtools::ghex::structured::grid",
            "gridtools::ghex::tl::mpi_tag",
            "gridtools::ghex::structured::regular::halo_generator<int, std::integral_constant<int, 3> >",
            "gridtools::ghex::structured::regular::domain_descriptor<int, std::integral_constant<int, 3> >",
            f"gridtools::layout_map<{', '.join(map(str, layout_map))}> ",
        )
        super(PatternContainer, self).__init__(
            type_spec,
            unwrap(context),
            unwrap(halo_generator),
            [unwrap(d) for d in domain_range],
        )
