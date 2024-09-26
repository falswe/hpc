#include <mpi.h>
#include <stdio.h>
#include <string.h>

int main(argc, argv) {
    int n_processes = 0;
    int id = 0;
    int v = 0;

    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &n_processes);
    MPI_Comm_rank(MPI_COMM_WORLD, &id);

    // loop-based with linear complexity
    if (id == 0) {
        scanf("%d", v);
        for (int i = 1; i < n_processes; i++) {
            MPI_Send(&v, 1, MPI_INT, i, 0, MPI_COMM_WORLD);
        }
    }
    else {
        for (int i = 1; i < n_processes; i++)
            MPI_Recv(&v, 1, MPI_INT, 1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    }
}
MPI_Finalize();
return 0;
}