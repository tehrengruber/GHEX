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

#include <cassert>
#include <sstream>

#include <gridtools/common/generic_metafunctions/for_each.hpp>
#include <gridtools/meta.hpp>

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include <ghex/bindings/python/utils/type_exporter.hpp>
#include <ghex/bindings/python/utils/demangle.hpp>
#include <ghex/bindings/python/type_list.hpp>

#include <ghex/structured/regular/field_descriptor.hpp>

namespace py = pybind11;

template <int... val>
using int_tuple_constant = gridtools::meta::list<std::integral_constant<int, val>...>;


namespace detail {
    using args_gpu = gridtools::meta::cartesian_product<
        gridtools::ghex::bindings::python::type_list::data_types,
        gridtools::meta::list<gridtools::ghex::gpu>,
        gridtools::meta::list<
            gridtools::ghex::structured::regular::domain_descriptor<int, std::integral_constant<int, 3>>
        >,
        gridtools::meta::list<
            ::gridtools::layout_map<0, 1, 2>,
            ::gridtools::layout_map<0, 2, 1>,
            ::gridtools::layout_map<1, 0, 2>,
            ::gridtools::layout_map<1, 2, 0>,
            ::gridtools::layout_map<2, 0, 1>,
            ::gridtools::layout_map<2, 1, 0>
        >
    >;

    template<typename T, typename Arch, typename DomainDescriptor, typename Layout>
    using field_descriptor_type = gridtools::ghex::structured::regular::field_descriptor<
        T, Arch, DomainDescriptor, Layout>;

    using field_descriptor_specializations_gpu = gridtools::meta::transform<gridtools::meta::rename<field_descriptor_type>::template apply, args_gpu>;
}

// todo: dim independent
template<typename T, typename Arch, typename DomainDescriptor, typename Layout>
struct type_exporter<gridtools::ghex::structured::regular::field_descriptor<T, Arch, DomainDescriptor, Layout>> {
    using field_descriptor_type = gridtools::ghex::structured::regular::field_descriptor<T, Arch, DomainDescriptor, Layout>;

    void operator() (pybind11::module_&, py::class_<field_descriptor_type> cls) {
        constexpr std::size_t dim = 3;

        using array_type = std::array<int, dim>;

        auto wrapper = [] (const DomainDescriptor& dom, py::buffer b, const array_type& offsets, const array_type& extents) {
            py::buffer_info info = b.request();

            if (info.format != py::format_descriptor<T>::format()) {
                std::stringstream error;
                error << "Incompatible format: expected a " << typeid(T).name() << " buffer.";
                throw pybind11::type_error(error.str());
            }

            auto ordered_strides = info.strides;
            std::sort(ordered_strides.begin(), ordered_strides.end(), [](int a, int b) { return a > b; });
            std::array<int, 3> layout_map;
            for (size_t i=0; i<3; ++i) {
                auto it = std::find(ordered_strides.begin(), ordered_strides.end(), info.strides[i]);
                layout_map[i] = std::distance(ordered_strides.begin(), it);
                if (layout_map[i] != Layout::at(i)) {
                    throw pybind11::type_error("Buffer has a different layout than specified.");
                }
            }

            if (info.ndim != 3)
                throw std::runtime_error("Incompatible buffer dimension.");

            return gridtools::ghex::wrap_field<Arch, Layout>(dom, static_cast<T*>(info.ptr), offsets, extents, info.strides);
        };

        cls.def(py::init(wrapper));
    }
};

GHEX_PYBIND11_EXPORT_TYPE(type_exporter, detail::field_descriptor_specializations_gpu)

/*
GHEX_PYBIND_EXPORT_IN_CURRENT_MODULE(field_descriptor_exporter,
    ghex4py::type_list::architecture_types,
    metal::list<domain_descriptor_type>,
    ghex4py::type_list::data_types,
    orders)

GHEX_PYBIND11_EXPORT_TYPE(gridtools::ghex::structured::regular::field_descriptor,
    ghex4py::type_list::architecture_types,
    metal::list<domain_descriptor_type>,
    ghex4py::type_list::data_types,
    orders)
*/