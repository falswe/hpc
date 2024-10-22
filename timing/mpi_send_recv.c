#include <limits.h>
#include <mpi.h>
#include <stdio.h>
#include <sys/time.h>

#define US_PER_S 1000000.0

double get_timespan_s(struct timeval *start, struct timeval *end) {
    double timespan;

    // Get the number of seconds, express it in microseconds
    timespan = (end->tv_sec - start->tv_sec) * US_PER_S;
    // Now add the microseconds
    timespan = (timespan + (end->tv_usec - start->tv_usec));

    return timespan / US_PER_S;
}

int main() {
    int id, n;

    struct timeval start, end;
    double time_taken, bandwidth;

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

            time_taken = get_timespan_s(&start, &end);

            bandwidth = ((i * (sizeof(int))) / time_taken) / 2.0;

            printf("%-15d %-15.6f %-12.6f\n", i, time_taken, bandwidth);
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
