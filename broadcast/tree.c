#include <mpi.h>
#include <stdio.h>

int main() {
    int n_processes = 0;
    int id = 0;
    int v = 0;

    MPI_Init(NULL, NULL);
    MPI_Comm_size(MPI_COMM_WORLD, &n_processes);
    MPI_Comm_rank(MPI_COMM_WORLD, &id);

    // tree-based with logarithmic complexity
    if (id == 0) {
        printf("send value %d to process %d\n", v, 1);
        MPI_Send(&v, 1, MPI_INT, 1, 0, MPI_COMM_WORLD);

        printf("send value %d to process %d\n", v, 2);
        MPI_Send(&v, 1, MPI_INT, 2, 0, MPI_COMM_WORLD);
    } else {
        if (id % 2 == 0) {
            MPI_Recv(&v, 1, MPI_INT, id / 2 - 1, 0, MPI_COMM_WORLD,
                     MPI_STATUS_IGNORE);
        } else {
            MPI_Recv(&v, 1, MPI_INT, id / 2, 0, MPI_COMM_WORLD,
                     MPI_STATUS_IGNORE);
        }
        printf("received value %d on process %d\n", v, id);

        if ((id * 2 + 1) < n_processes) {
            printf("send value %d to process %d\n", v, id * 2 + 1);
            MPI_Send(&v, 1, MPI_INT, id * 2 + 1, 0, MPI_COMM_WORLD);
        }
        if ((id * 2 + 2) < n_processes) {
            printf("send value %d to process %d\n", v, id * 2 + 2);
            MPI_Send(&v, 1, MPI_INT, id * 2 + 2, 0, MPI_COMM_WORLD);
        }
    }

    MPI_Finalize();
    return 0;
}