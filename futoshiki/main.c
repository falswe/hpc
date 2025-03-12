#include <mpi.h>
#include <omp.h>
#include <stdio.h>
#include <string.h>

#include "comparison.h"
#include "futoshiki.h"

int main(int argc, char* argv[]) {
    // Initialize MPI
    MPI_Init(&argc, &argv);

    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    // Process command line arguments - only the root process prints usage info
    if (argc != 2 && argc != 3) {
        if (rank == 0) {
            printf("Usage: %s <puzzle_file> [-c|-n]\n", argv[0]);
            printf("  -c: comparison mode (run both with and without precoloring)\n");
            printf("  -n: disable precoloring\n");
        }
        MPI_Finalize();
        return 1;
    }

    if (argc == 3 && strcmp(argv[2], "-c") == 0) {
        run_comparison_parallel(argv[1], rank, size);
    } else {
        bool use_precoloring = (argc == 2 || strcmp(argv[2], "-n") != 0);
        SolverStats stats = solve_puzzle_parallel(argv[1], use_precoloring, rank == 0, rank, size);

        // Only root process prints the results
        if (rank == 0) {
            print_stats(&stats, "");
        }
    }

    MPI_Finalize();
    return 0;
}