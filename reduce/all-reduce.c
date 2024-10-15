#include <mpi.h>
#include <stdio.h>

int main() {
    int n_processes = 0;
    int id = 0;
    int v = 0;
    int sum = 0;

    MPI_Init(NULL, NULL);
    MPI_Comm_size(MPI_COMM_WORLD, &n_processes);
    MPI_Comm_rank(MPI_COMM_WORLD, &id);

    if (id == 0) {
        for (int i = 1; i < n_processes; i++) {
            MPI_Recv(&v, 1, MPI_INT, i, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            printf("received value %d from process %d on process %d\n", v, i,
                   id);
            sum += v;
        }

        MPI_Send(&sum, 1, MPI_INT, 1, 0, MPI_COMM_WORLD);
        printf("send value %d from process %d to process %d\n", sum, id, 1);

        MPI_Send(&sum, 1, MPI_INT, 2, 0, MPI_COMM_WORLD);
        printf("send value %d from process %d to process %d\n", sum, id, 2);

    } else {
        // sending out own id
        v = id;
        MPI_Send(&v, 1, MPI_INT, 0, 0, MPI_COMM_WORLD);
        printf("send value %d from process %d to process %d\n", v, id, 0);

        if (id % 2 == 0) {
            MPI_Recv(&sum, 1, MPI_INT, id / 2 - 1, 0, MPI_COMM_WORLD,
                     MPI_STATUS_IGNORE);
            printf("received value %d from process %d on process %d\n", sum,
                   id / 2 - 1, id);
        } else {
            MPI_Recv(&sum, 1, MPI_INT, id / 2, 0, MPI_COMM_WORLD,
                     MPI_STATUS_IGNORE);
            printf("received value %d from process %d on process %d\n", sum,
                   id / 2, id);
        }

        if ((id * 2 + 1) < n_processes) {
            MPI_Send(&sum, 1, MPI_INT, id * 2 + 1, 0, MPI_COMM_WORLD);
            printf("send value %d from process %d to process %d\n", sum, id,
                   id * 2 + 1);
        }
        if ((id * 2 + 2) < n_processes) {
            MPI_Send(&sum, 1, MPI_INT, id * 2 + 2, 0, MPI_COMM_WORLD);
            printf("send value %d from process %d to process %d\n", sum, id,
                   id * 2 + 2);
        }
    }

    MPI_Finalize();
    return 0;
}