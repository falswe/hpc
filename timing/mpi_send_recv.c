#include <mpi.h>
#include <stdio.h>
#include <sys/time.h>
#include <limits.h>

int main() {
    int id, n;

    struct timeval start, end;
    double temp_time_taken, time_taken_seconds;
    double bandwidth;

    int *v;

    MPI_Init(NULL, NULL);

    MPI_Comm_rank(MPI_COMM_WORLD, &id);
    MPI_Comm_size(MPI_COMM_WORLD, &n);

    if (id == 0) {
        printf("%-15s %-15s %-20s\n", "n", "time (sec)", "bandwidth (B/s)");
    }
    for (int i = 1; i < INT_MAX; i = i * 2) {
        if (id == 0) {
            v = malloc(i * (sizeof(int)));

            gettimeofday(&start, NULL);

            MPI_Send(v, i, MPI_INT, 1, 0, MPI_COMM_WORLD);
            MPI_Recv(v, i, MPI_INT, 1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

            gettimeofday(&end, NULL);

            free(v);

            // Get the number of seconds, express it in microseconds
            temp_time_taken = (end.tv_sec - start.tv_sec) * 1e6;
            // Now add the microseconds
            temp_time_taken = (temp_time_taken + (end.tv_usec - start.tv_usec));
            // convert back to seconds
            time_taken_seconds = temp_time_taken / 1000000.0;

            // calculate bandwidth
            bandwidth = ((i * (sizeof(int))) / 2) / time_taken_seconds;

            printf("%-15d %-15.6f %-12.6f\n", i, time_taken_seconds, bandwidth);
        } else {
            v = malloc(i * (sizeof(int)));

            MPI_Recv(v, i, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            MPI_Send(v, i, MPI_INT, 0, 0, MPI_COMM_WORLD);

            free(v);
        }
    }

    MPI_Finalize();
    return 0;
}
