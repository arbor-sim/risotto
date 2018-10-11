#include <iostream>
#include <mpi.h>
#include <arbor/context.hpp>
#include <arbor/version.hpp>

#ifndef ARB_MPI_ENABLED
#error "Requires MPI-enabled arbor build"
#endif

int main(int argc, char** argv) {
    MPI_Init(&argc, &argv);
    arb::context ctx = arb::make_context(arb::proc_allocation(1, -1), MPI_COMM_WORLD);
    std::cout << num_ranks(ctx) << "\n";
    MPI_Finalize();
}
