#include <math.h>
#include <omp.h>
#include <stdio.h>
#include <stdlib.h>

double calculate_pi(int n_terms, int n_threads) {
    double factor = 1.0;
    double sum = 0.0;

#pragma omp parallel for num_threads(n_threads) \
    reduction(+ : sum) private(factor)
    for (int alternate = 0; alternate < n_terms; alternate++) {
        factor = 1.0 / (2.0 * alternate + 1);
        if (alternate % 2 != 0) {
            factor = -factor;
        }
        sum += factor;
    }

    return 4.0 * sum;
}

int main() {
    int n_terms = 1000000;                  // Number of terms in the series
    int n_threads = omp_get_max_threads();  // Use maximum available threads

    printf("Calculating π using %d threads...\n", n_threads);

    double pi_approx = calculate_pi(n_terms, n_threads);

    printf("Approximation of π: %.15f\n", pi_approx);
    printf("Actual value of π: 3.141592653589793\n");
    printf("Absolute error: %.15f\n", fabs(pi_approx - 3.141592653589793));

    return 0;
}