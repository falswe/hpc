#include <mpi.h>
#include <stdio.h>
#include <sys/time.h>

int main() {
    int id, n;

    struct timeval start, end;
    double temp_time_taken, time_taken;

    int v = 42;

    MPI_Init(NULL, NULL);

    MPI_Comm_rank(MPI_COMM_WORLD, &id);
    MPI_Comm_size(MPI_COMM_WORLD, &n);

    if (id == 0) {
        gettimeofday(&start, NULL);

        MPI_Send(&v, 1, MPI_INT, 1, 0, MPI_COMM_WORLD);
        MPI_Recv(&v, 1, MPI_INT, 1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

        gettimeofday(&end, NULL);

        // Get the number of seconds, express it in microseconds
        temp_time_taken = (end.tv_sec - start.tv_sec) * 1e6;

        // Now add the microseconds
        temp_time_taken = (temp_time_taken + (end.tv_usec - start.tv_usec));
        time_taken = temp_time_taken / 1e6;

        printf("time (sec)");
        printf("%ld\n", time_taken);
    } else {
        MPI_Recv(&v, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        MPI_Send(&v, 1, MPI_INT, 0, 0, MPI_COMM_WORLD);
    }

    MPI_Finalize();
    return 0;
}
