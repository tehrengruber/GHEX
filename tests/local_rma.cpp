/*
 * GridTools
 *
 * Copyright (c) 2014-2020, ETH Zurich
 * All rights reserved.
 *
 * Please, refer to the LICENSE file in the root directory.
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#include <gtest/gtest.h>
#include <thread>
#include <array>
#include <gridtools/common/array.hpp>

#include <ghex/threads/atomic/primitives.hpp>
#include <ghex/threads/std_thread/primitives.hpp>
#ifndef GHEX_TEST_USE_UCX
#include <ghex/transport_layer/mpi/context.hpp>
using transport = gridtools::ghex::tl::mpi_tag;
using threading = gridtools::ghex::threads::std_thread::primitives;
#else
#include <ghex/transport_layer/ucx/context.hpp>
using transport = gridtools::ghex::tl::ucx_tag;
using threading = gridtools::ghex::threads::std_thread::primitives;
#endif

#include <ghex/structured/remote_thread_range.hpp>
#include <ghex/structured/bulk_communication_object.hpp>
#include <ghex/structured/regular/domain_descriptor.hpp>
#include <ghex/structured/regular/field_descriptor.hpp>
#include <ghex/structured/regular/halo_generator.hpp>

//#include <iomanip>
//#include <future>
//#ifdef __CUDACC__
//#include <gridtools/common/cuda_util.hpp>
//#include <gridtools/common/host_device.hpp>
//#endif


struct simulation_1
{
    template<typename T, std::size_t N>
    using array_type = gridtools::array<T,N>;
    using T1 = double;
    using T2 = float;
    using T3 = int;
    using TT1 = array_type<T1,3>;
    using TT2 = array_type<T2,3>;
    using TT3 = array_type<T3,3>;

    using context_type = gridtools::ghex::tl::context<transport,threading>;
    using context_ptr_type = std::unique_ptr<context_type>;
    using domain_descriptor_type = gridtools::ghex::structured::regular::domain_descriptor<int,3>;
    using halo_generator_type = gridtools::ghex::structured::regular::halo_generator<int,3>;
    template<typename T, typename Arch, int... Is>
    using field_descriptor_type  = gridtools::ghex::structured::regular::field_descriptor<T,Arch,domain_descriptor_type, Is...>;
        

    // decomposition: 4 domains in x-direction, 1 domain in z-direction, rest in y-direction
    //                each MPI rank owns two domains: either first or last two domains in x-direction
    //
    //          +---------> x
    //          |
    //          |     +------<0>------+------<1>------+
    //          |     | +----+ +----+ | +----+ +----+ |
    //          v     | |  0 | |  1 | | |  2 | |  3 | |
    //                | +----+ +----+ | +----+ +----+ |
    //          y     +------<2>------+------<3>------+
    //                | +----+ +----+ | +----+ +----+ |
    //                | |  4 | |  5 | | |  6 | |  7 | |
    //                | +----+ +----+ | +----+ +----+ |
    //                +------<4>------+------<5>------+
    //                | +----+ +----+ | +----+ +----+ |
    //                | |  8 | |  9 | | | 10 | | 11 | |
    //                . .    . .    . . .    . .    . .
    //                . .    . .    . . .    . .    . .
    //

    context_ptr_type context_ptr;
    context_type& context;
    const std::array<int,3> local_ext;
    const std::array<bool,3> periodic;
    const std::array<int,3> g_first;
    const std::array<int,3> g_last;
    const std::array<int,3> offset;
    const std::array<int,3> local_ext_buffer;
    const int max_memory;
    std::vector<TT1> field_1a_raw;
    std::vector<TT1> field_1b_raw;
    std::vector<TT2> field_2a_raw;
    std::vector<TT2> field_2b_raw;
    std::vector<TT3> field_3a_raw;
    std::vector<TT3> field_3b_raw;
    std::vector<domain_descriptor_type> local_domains;
    std::array<int,6> halos;
    halo_generator_type halo_gen;
    using pattern_type = std::remove_reference_t<decltype(
        gridtools::ghex::make_pattern<gridtools::ghex::structured::grid>(context, halo_gen, local_domains))>;
    pattern_type pattern;
    field_descriptor_type<TT1, gridtools::ghex::cpu, 2, 1, 0> field_1a;
    field_descriptor_type<TT1, gridtools::ghex::cpu, 2, 1, 0> field_1b;
    field_descriptor_type<TT2, gridtools::ghex::cpu, 2, 1, 0> field_2a;
    field_descriptor_type<TT2, gridtools::ghex::cpu, 2, 1, 0> field_2b;
    field_descriptor_type<TT3, gridtools::ghex::cpu, 2, 1, 0> field_3a;
    field_descriptor_type<TT3, gridtools::ghex::cpu, 2, 1, 0> field_3b;
    typename context_type::communicator_type comm;
    std::vector<typename context_type::communicator_type> comms;
    std::vector<gridtools::ghex::bulk_communication_object> cos;
    bool mt;

    simulation_1(bool multithread = false)
    : context_ptr{ gridtools::ghex::tl::context_factory<transport,threading>::create(multithread ? 2 : 1, MPI_COMM_WORLD) }
    , context{*context_ptr}
    , local_ext{4,3,2}
    //, local_ext{14,13,12}
    , periodic{true,true,true}
    , g_first{0,0,0}
    , g_last{local_ext[0]*4-1, ((context.size()-1)/2+1)*local_ext[1]-1, local_ext[2]-1}
    , offset{3,3,3}
    , local_ext_buffer{local_ext[0]+2*offset[0], local_ext[1]+2*offset[1], local_ext[2]+2*offset[2]}
    , max_memory{local_ext_buffer[0]*local_ext_buffer[1]*local_ext_buffer[2]}
    , field_1a_raw(max_memory)
    , field_1b_raw(max_memory)
    , field_2a_raw(max_memory)
    , field_2b_raw(max_memory)
    , field_3a_raw(max_memory)
    , field_3b_raw(max_memory)
    , local_domains{ 
        domain_descriptor_type{
            context.rank()*2,
            std::array<int,3>{ ((context.rank()%2)*2  )*local_ext[0],   (context.rank()/2  )*local_ext[1],                0},
            std::array<int,3>{ ((context.rank()%2)*2+1)*local_ext[0]-1, (context.rank()/2+1)*local_ext[1]-1, local_ext[2]-1}},
        domain_descriptor_type{
            context.rank()*2+1,
            std::array<int,3>{ ((context.rank()%2)*2+1)*local_ext[0],   (context.rank()/2  )*local_ext[1],             0},
            std::array<int,3>{ ((context.rank()%2)*2+2)*local_ext[0]-1, (context.rank()/2+1)*local_ext[1]-1, local_ext[2]-1}}}
    , halos{2,2,2,2,2,2}
    , halo_gen(g_first, g_last, halos, periodic)
    , pattern{gridtools::ghex::make_pattern<gridtools::ghex::structured::grid>(context, halo_gen, local_domains)}
    , field_1a{gridtools::ghex::wrap_field<gridtools::ghex::cpu,2,1,0>(local_domains[0], field_1a_raw.data(), offset, local_ext_buffer)}
    , field_1b{gridtools::ghex::wrap_field<gridtools::ghex::cpu,2,1,0>(local_domains[1], field_1b_raw.data(), offset, local_ext_buffer)}
    , field_2a{gridtools::ghex::wrap_field<gridtools::ghex::cpu,2,1,0>(local_domains[0], field_2a_raw.data(), offset, local_ext_buffer)}
    , field_2b{gridtools::ghex::wrap_field<gridtools::ghex::cpu,2,1,0>(local_domains[1], field_2b_raw.data(), offset, local_ext_buffer)}
    , field_3a{gridtools::ghex::wrap_field<gridtools::ghex::cpu,2,1,0>(local_domains[0], field_3a_raw.data(), offset, local_ext_buffer)}
    , field_3b{gridtools::ghex::wrap_field<gridtools::ghex::cpu,2,1,0>(local_domains[1], field_3b_raw.data(), offset, local_ext_buffer)}
    , comm{ context.get_serial_communicator() }
    , mt{multithread}
    {
        fill_values(local_domains[0], field_1a);
        fill_values(local_domains[1], field_1b);
        fill_values(local_domains[0], field_2a);
        fill_values(local_domains[1], field_2b);
        fill_values(local_domains[0], field_3a);
        fill_values(local_domains[1], field_3b);
    
        if (!mt)
        {
            comms.push_back(context.get_communicator(context.get_token()));
            cos.push_back(
                gridtools::ghex::make_bulk_co<gridtools::ghex::structured::remote_thread_range_generator>(
                    comms[0], pattern, field_1a, field_1b, field_2a, field_2b, field_3a, field_3b));
        } else {
            comms.push_back(context.get_communicator(context.get_token()));
            comms.push_back(context.get_communicator(context.get_token()));
            cos.resize(2);
            std::vector<std::thread> threads;
            threads.push_back(std::thread{[this](){
                cos[0] = gridtools::ghex::make_bulk_co<gridtools::ghex::structured::remote_thread_range_generator>(
                    comms[0], pattern, field_1a, field_2a, field_3a);
            }});
            threads.push_back(std::thread{[this](){
                cos[1] = gridtools::ghex::make_bulk_co<gridtools::ghex::structured::remote_thread_range_generator>(
                    comms[1], pattern, field_1b, field_2b, field_3b);
            }});
            for (auto& t : threads) t.join();
        }
    }

    void exchange()
    {
        if (!mt)
        {
            cos[0].exchange();
        }
        else
        {
            std::vector<std::thread> threads;
            threads.push_back(std::thread{[this]() -> void { cos[0].exchange(); }});
            threads.push_back(std::thread{[this]() -> void { cos[1].exchange(); }});
            for (auto& t : threads) t.join();
        }
    }

    bool check()
    {
        bool passed =true;
        passed = passed && test_values(local_domains[0], field_1a);
        passed = passed && test_values(local_domains[1], field_1b);
        passed = passed && test_values(local_domains[0], field_2a);
        passed = passed && test_values(local_domains[1], field_2b);
        passed = passed && test_values(local_domains[0], field_3a);
        passed = passed && test_values(local_domains[1], field_3b);
        return passed;
    }

private:
    template<typename Field>
    void fill_values(const domain_descriptor_type& d, Field& f)
    {
        using T = typename Field::value_type::value_type;
        int xl = 0;
        for (int x=d.first()[0]; x<=d.last()[0]; ++x, ++xl)
        {
            int yl = 0;
            for (int y=d.first()[1]; y<=d.last()[1]; ++y, ++yl)
            {
                int zl = 0;
                for (int z=d.first()[2]; z<=d.last()[2]; ++z, ++zl)
                {
                    f(xl,yl,zl) = array_type<T,3>{(T)x,(T)y,(T)z};
                }
            }
        }
    }
    
    template<typename Field>
    bool test_values(const domain_descriptor_type& d, const Field& f)
    {
        using T = typename Field::value_type::value_type;
        bool passed = true;
        const int i = d.domain_id()%2;
        int rank = comm.rank();
        int size = comm.size();
    
        int xl = -halos[0];
        int hxl = halos[0];
        int hxr = halos[1];
        // hack begin: make it work with 1 rank (works with even number of ranks otherwise)
        if (i==0 && size==1)//comm.rank()%2 == 0 && comm.rank()+1 == comm.size())
        {
            xl = 0;
            hxl = 0;
        }
        if (i==1 && size==1)//comm.rank()%2 == 0 && comm.rank()+1 == comm.size())
        {
            hxr = 0;
        }
        // hack end
        for (int x=d.first()[0]-hxl; x<=d.last()[0]+hxr; ++x, ++xl)
        {
            if (i==0 && x<d.first()[0] && !periodic[0]) continue;
            if (i==1 && x>d.last()[0]  && !periodic[0]) continue;
            T x_wrapped = (((x-g_first[0])+(g_last[0]-g_first[0]+1))%(g_last[0]-g_first[0]+1) + g_first[0]);
            int yl = -halos[2];
            for (int y=d.first()[1]-halos[2]; y<=d.last()[1]+halos[3]; ++y, ++yl)
            {
                if (d.domain_id()<2 &&      y<d.first()[1] && !periodic[1]) continue;
                if (d.domain_id()>size-3 && y>d.last()[1]  && !periodic[1]) continue;
                T y_wrapped = (((y-g_first[1])+(g_last[1]-g_first[1]+1))%(g_last[1]-g_first[1]+1) + g_first[1]);
                int zl = -halos[4];
                for (int z=d.first()[2]-halos[4]; z<=d.last()[2]+halos[5]; ++z, ++zl)
                {
                    if (z<d.first()[2] && !periodic[2]) continue;
                    if (z>d.last()[2]  && !periodic[2]) continue;
                    T z_wrapped = (((z-g_first[2])+(g_last[2]-g_first[2]+1))%(g_last[2]-g_first[2]+1) + g_first[2]);
    
                    const auto& value = f(xl,yl,zl);
                    if(value[0]!=x_wrapped || value[1]!=y_wrapped || value[2]!=z_wrapped)
                    {
                        passed = false;
                        std::cout
                        << "(" << xl << ", " << yl << ", " << zl << ") values found != expected: "
                        << "(" << value[0] << ", " << value[1] << ", " << value[2] << ") != "
                        << "(" << x_wrapped << ", " << y_wrapped << ", " << z_wrapped << ") " //<< std::endl;
                        << i << "  " << rank << std::endl;
                    }
                }
            }
        }
        return passed;
    }
};

TEST(local_rma, single)
{
    simulation_1 sim(false);
    sim.exchange();
    sim.exchange();
    sim.exchange();
    EXPECT_TRUE(sim.check());
}

TEST(local_rma, multi)
{
    simulation_1 sim(true);
    sim.exchange();
    sim.exchange();
    sim.exchange();
    EXPECT_TRUE(sim.check());
}
