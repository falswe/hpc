#include <omp.h>
#include <stdio.h>

void fibonacci_sequential(int n, int fibo[]) {
    // Initialize first two numbers
    fibo[0] = fibo[1] = 1;

    // Calculate subsequent numbers
    for (int i = 2; i < n; i++) {
        fibo[i] = fibo[i - 1] + fibo[i - 2];
    }
}

void fibonacci_parallel(int n, int fibo[]) {
    // Initialize first two numbers
    fibo[0] = fibo[1] = 1;

// Calculate subsequent numbers in parallel
#pragma omp parallel for num_threads(2)
    for (int i = 2; i < n; i++) {
        fibo[i] = fibo[i - 1] + fibo[i - 2];
    }
}

int main() {
    const int N = 10;  // Calculate first 10 Fibonacci numbers
    int fibo_seq[N];
    int fibo_parallel[N];

    // Calculate using sequential method
    printf("Sequential Fibonacci:\n");
    fibonacci_sequential(N, fibo_seq);
    for (int i = 0; i < N; i++) {
        printf("%d ", fibo_seq[i]);
    }
    printf("\n");

    // Calculate using parallel method
    printf("Parallel Fibonacci:\n");
    fibonacci_parallel(N, fibo_parallel);
    for (int i = 0; i < N; i++) {
        printf("%d ", fibo_parallel[i]);
    }
    printf("\n");

    return 0;
}