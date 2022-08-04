import numpy as np
import pytest

from ghex.structured.regular import (
    CommunicationObject,
    DomainDescriptor,
    FieldDescriptor,
    HaloGenerator,
    PatternContainer,
)
from ghex.tl import Context
from ghex.utils.grid import IndexSpace, UnitRange

from fixtures.mpi import mpi_cart_comm, ghex_cart_context

# Domain configuration
Nx = 10
Ny = 10
Nz = 2

haloss = [
    # (0, 0, 0),
    (1, 0, 0),
    (1, 2, 3),
    ((1, 0), (0, 0), (0, 0)),
    ((1, 0), (0, 2), (2, 2)),
]

layout_maps = ((0, 1, 2), (0, 2, 1), (1, 0, 2), (1, 2, 0), (2, 0, 1), (2, 1, 0))


def test_context(mpi_cart_comm):
    context = Context(mpi_cart_comm)

    with pytest.raises(TypeError, match=r"must be `mpi4py.MPI.Comm`, not `<class 'str'>`"):
        context = Context("invalid")


def test_domain_descriptor(mpi_cart_comm):
    context = Context(mpi_cart_comm)

    coords = mpi_cart_comm.Get_coords(mpi_cart_comm.Get_rank())

    sub_domain_indices = (
        UnitRange(coords[0] * Nx, (coords[0] + 1) * Nx)
        * UnitRange(coords[1] * Ny, (coords[1] + 1) * Ny)
        * UnitRange(coords[2] * Nz, (coords[2] + 1) * Nz)
    )

    domain_desc = DomainDescriptor(context.rank(), sub_domain_indices)

    assert domain_desc.domain_id() == context.rank()
    assert domain_desc.first() == sub_domain_indices[0, 0, 0]
    assert domain_desc.last() == sub_domain_indices[-1, -1, -1]


@pytest.mark.parametrize("halos", haloss)
def test_halo_gen_construction(mpi_cart_comm, halos):
    dims = mpi_cart_comm.dims
    glob_domain_indices = (
        UnitRange(0, dims[0] * Nx) * UnitRange(0, dims[1] * Ny) * UnitRange(0, dims[2] * Nz)
    )

    halo_gen = HaloGenerator(glob_domain_indices, halos, (False, False, False))


@pytest.mark.parametrize("halos", haloss)
def test_halo_gen_call(mpi_cart_comm, halos):
    context = Context(mpi_cart_comm)

    periodicity = (False, False, False)

    p_coord = tuple(mpi_cart_comm.Get_coords(mpi_cart_comm.Get_rank()))

    # setup grid
    global_grid = IndexSpace.from_sizes(Nx, Ny, Nz)
    sub_grids = global_grid.decompose(mpi_cart_comm.dims)
    sub_grid = sub_grids[p_coord]  # sub-grid in global coordinates
    owned_indices = sub_grid.subset["definition"]
    sub_grid.add_subset("halo", owned_indices.extend(*halos).without(owned_indices))

    # construct HaloGenerator
    halo_gen = HaloGenerator(global_grid.subset["definition"], halos, periodicity)

    domain_desc = DomainDescriptor(context.rank(), owned_indices)

    # test generated halos
    halo_indices = halo_gen(domain_desc)
    assert sub_grid.subset["halo"] == halo_indices.global_
    # assert sub_grid.subset["halo"] == halo_indices.local.translate(...)


@pytest.mark.parametrize("halos", haloss)
def test_domain_descriptor_grid(mpi_cart_comm, halos):
    p_coord = tuple(mpi_cart_comm.Get_coords(mpi_cart_comm.Get_rank()))

    global_grid = IndexSpace.from_sizes(Nx, Ny, Nz)
    sub_grids = global_grid.decompose(mpi_cart_comm.dims)
    sub_grid = sub_grids[p_coord]  # sub-grid in global coordinates
    owned_indices = sub_grid.subset["definition"]
    sub_grid.add_subset("halo", owned_indices.extend(*halos).without(owned_indices))

    context = Context(mpi_cart_comm)
    domain_desc = DomainDescriptor(context.rank(), owned_indices)

    assert domain_desc.domain_id() == context.rank()
    assert domain_desc.first() == owned_indices.bounds[0, 0, 0]
    assert domain_desc.last() == owned_indices.bounds[-1, -1, -1]


@pytest.mark.parametrize("layout_map", layout_maps)
def test_pattern(mpi_cart_comm, layout_map):
    Nx, Ny, Nz = 2 * 260, 260, 2
    # Nx, Ny, Nz = 2 * 260, 260, 1
    halos = ((2, 1), (1, 1), (0, 0))
    periodicity = (True, True, False)

    p_coord = tuple(mpi_cart_comm.Get_coords(mpi_cart_comm.Get_rank()))
    global_grid = IndexSpace.from_sizes(Nx, Ny, Nz)
    sub_grids = global_grid.decompose(mpi_cart_comm.dims)
    sub_grid = sub_grids[p_coord]  # sub-grid in global coordinates
    owned_indices = sub_grid.subset["definition"]
    sub_grid.add_subset("halo", owned_indices.extend(*halos).without(owned_indices))

    memory_local_grid = sub_grid.translate(*(-origin_l for origin_l in sub_grid.bounds[0, 0, 0]))

    # for coord, local_grid in local_grids.items():
    # print(f"p_coord: {p_coord}, memory_local_grid: ", memory_local_grid)
    # print(f"p_coord: {p_coord}, local_local_grid: ", local_local_grid)
    print(f"p_coord: {p_coord}, local_grid: ", sub_grid)
    #    print(local_grid.bounds)

    # domain descriptor
    context = Context(mpi_cart_comm)
    domain_desc = DomainDescriptor(context.rank(), owned_indices)

    # halo generator
    halo_gen = HaloGenerator(global_grid.subset["definition"], halos, periodicity)
    # print("local_local: ", local_local_grid.subset["definition"])

    pattern = PatternContainer("cpu", layout_map, context, halo_gen, (domain_desc,))

    co = CommunicationObject("cpu", layout_map, context.get_communicator())

    def get_strides(field: np.ndarray, layout_map: tuple[int, ...]) -> tuple[int, ...]:
        shape = field.shape
        itemsize = np.dtype(field.dtype).itemsize
        cumulative_stride = itemsize
        sorted_layout_map = reversed(tuple(range(0, len(layout_map))))
        strides = [None] * len(layout_map)
        for target in sorted_layout_map:
            index = layout_map.index(target)
            strides[index] = cumulative_stride
            cumulative_stride *= shape[index]
        return tuple(strides)

    def make_field():
        field_1 = np.zeros(memory_local_grid.bounds.shape, dtype=np.float64)  # todo: , order='F'
        strides = get_strides(field_1, layout_map)
        field_1 = np.lib.stride_tricks.as_strided(field_1, strides=strides)
        gfield_1 = FieldDescriptor(
            "cpu",
            domain_desc,
            field_1,
            memory_local_grid.subset["definition"][0, 0, 0],
            memory_local_grid.bounds.shape,
        )
        return field_1, gfield_1

    field_1, gfield_1 = make_field()
    field_2, gfield_2 = make_field()
    field_3, gfield_3 = make_field()
    fields = (field_1, field_2, field_3)
    gfields = (gfield_1, gfield_2, gfield_3)
    for p_dim, p_coord_l in enumerate(p_coord):
        fields[p_dim][:, :, :] = p_coord_l

        res = co.exchange(pattern.__wrapped__(gfields[p_dim].__wrapped__))
        res.wait()

    rank_field, grank_field = make_field()
    rank_field[:, :, :] = context.rank()
    res = co.exchange(
        pattern.__wrapped__(grank_field.__wrapped__)
    )  # arch, dtype. exchange of fields living on cpu+gpu possible
    res.wait()
    # todo: co.bexchange

    for m_idx, local_idx in zip(memory_local_grid.bounds, sub_grid.bounds):
        value_owner_coord = tuple(int(fields[dim][m_idx]) for dim in range(0, 3))
        value_owner_rank = mpi_cart_comm.Get_cart_rank(value_owner_coord)
        if all(
            l >= 0 and l <= global_grid.subset["definition"][-1, -1, -1][d]
            for d, l in enumerate(local_idx)
        ):
            assert local_idx in sub_grids[value_owner_coord].subset["definition"]

            assert rank_field[m_idx] == value_owner_rank

    print("post_ex:")
    print(rank_field[:, :, 0])


if __name__ == "__main__":
    pytest.main([__file__])