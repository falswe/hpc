#include <omp.h>
#include <stdio.h>
#include <stdlib.h>

// Mathematical function that you want to integrate using the trapezoidal rule
double f(double x) { return x * x; }

void trapezoid(double endpoint_l, double endpoint_r, int n_trapezoids,
               double* result_global) {
    double h, x, result_thread;
    double local_a, local_b;
    int i, local_n;
    int id_thread = omp_get_thread_num();
    int n_threads = omp_get_num_threads();

    h = (endpoint_r - endpoint_l) / n_trapezoids;
    local_n = n_trapezoids / n_threads;
    local_a = endpoint_l + id_thread * local_n * h;
    local_b = local_a + local_n * h;
    result_thread = (f(local_a) + f(local_b)) / 2.0;

    for (i = 1; i <= local_n - 1; i++) {
        x = local_a + i * h;
        result_thread += f(x);
    }
    result_thread = result_thread * h;

#pragma omp critical
    *result_global += result_thread;
}

int main(int argc, char* argv[]) {
    double result_global = 0.0;
    double endpoint_l = -1;
    double endpoint_r = 1;
    int n_trapezoids = 4;
    int n_threads;

    n_threads = strtol(argv[1], NULL, 10);
    // printf("Enter left, right endpoint and total number of trapezoids\n");
    // scanf("%lf %lf %d", &endpoint_l, &endpoint_r, &n_trapezoids);

#pragma omp parallel num_threads(n_threads)
    trapezoid(endpoint_l, endpoint_r, n_trapezoids, &result_global);

    printf("With total number of trapezoids = %d, our estimate\n",
           n_trapezoids);
    printf("of the integral from %f to %f = %.14e\n", endpoint_l, endpoint_r,
           result_global);

    return 0;
}
