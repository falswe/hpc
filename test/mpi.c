#include <mpi.h>
#include <stdio.h>

int main() {
    MPI_Init(NULL, NULL);

    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    printf("Process %d of %d is running\n", rank, size);

    MPI_Finalize();
    return 0;
}
