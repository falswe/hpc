#include <mpi.h>
#include <stdio.h>

int main() {
    int n_processes = 0;
    int id = 0;
    int v = 0;

    MPI_Init(NULL, NULL);
    MPI_Comm_size(MPI_COMM_WORLD, &n_processes);
    MPI_Comm_rank(MPI_COMM_WORLD, &id);

    // ring-based with linear complexity
    if (id == 0) {
        printf("send value %d to process %d\n", v, 1);
        MPI_Send(&v, 1, MPI_INT, 1, 0, MPI_COMM_WORLD);
    } else {
        MPI_Recv(&v, 1, MPI_INT, id - 1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        printf("received value %d on process %d\n", v, id - 1);

        if (id != (n_processes - 1)) {
            printf("send value %d to process %d\n", v, id + 1);
            MPI_Send(&v, 1, MPI_INT, id + 1, 0, MPI_COMM_WORLD);
        }
    }

    MPI_Finalize();
    return 0;
}