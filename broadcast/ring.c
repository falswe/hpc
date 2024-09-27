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

  do {
    if (id == 0) {
      scanf("%d", v);
      MPI_Send(&v, 1, MPI_INT, id + 1, 0, MPI_COMM_WORLD);
    } else {
      MPI_Recv(&v, 1, MPI_INT, id - 1, 0, MPI_COMM_WORLD, &status);
      if (id < n_processes - 1) {
        // something is missing here
      }
    }
  }
  MPI_Finalize();
  return 0;
}