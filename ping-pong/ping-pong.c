#include <mpi.h>
#include <stdio.h>

int main() {
    int n_processes = 0;
    int id = 0;
    int v = 19;

    MPI_Init(NULL, NULL);
    MPI_Comm_size(MPI_COMM_WORLD, &n_processes);
    MPI_Comm_rank(MPI_COMM_WORLD, &id);

    if ((n_processes % 2 == 0) || (id != n_processes - 1)) {
        if (id % 2 == 0) {
            while (v > 0) {
                MPI_Recv(&v, 1, MPI_INT, id + 1, 0, MPI_COMM_WORLD,
                         MPI_STATUS_IGNORE);
                printf("received value %d from process %d on process %d\n", v,
                       id + 1, id);

                v--;

                MPI_Send(&v, 1, MPI_INT, id + 1, 0, MPI_COMM_WORLD);
                printf("send value %d from process %d to process %d\n", v, id,
                       id + 1);
            }

        } else {
            while (v > 0) {
                MPI_Send(&v, 1, MPI_INT, id - 1, 0, MPI_COMM_WORLD);
                printf("send value %d from process %d to process %d\n", v, id,
                       id - 1);

                MPI_Recv(&v, 1, MPI_INT, id - 1, 0, MPI_COMM_WORLD,
                         MPI_STATUS_IGNORE);
                printf("received value %d from process %d on process %d\n", v,
                       id - 1, id);

                v--;
            }
        }
    } else {
        printf("process %d is not doing anything\n", id);
    }

    MPI_Finalize();
}