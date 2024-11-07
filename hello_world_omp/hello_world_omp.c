#include <omp.h>
#include <stdio.h>
#include <stdlib.h>

void hello() {
    int id_thread = omp_get_thread_num();
    int n_threads = omp_get_num_threads();

    printf("Hello from thread %d of %d\n", id_thread, n_threads);
}

int main(int argc, char* argv[]) {
    int n_threads = strtol(argv[1], NULL, 10);

    printf("Hello from the serial world\n");

#pragma omp parallel num_threads(n_threads)
    hello();

    printf("Back to the serial world\n");
    return 0;
}