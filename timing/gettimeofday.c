#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>

int main(int argc, char **argv) {
    int i = 0;
    if (argc < 2) {
        printf("USAGE: %s loop-iterations\n", argv[0]);
        return 1;
    }

    int iterations = atoi(argv[1]);

    struct timeval start, end;

    gettimeofday(&start, NULL);

    for (i = 0; i < iterations; i++) {
        // Empty loop body
    }

    gettimeofday(&end, NULL);

    printf("%ld\n", ((end.tv_sec * 1000000 + end.tv_usec) -
                     (start.tv_sec * 1000000 + start.tv_usec)));

    return 0;
}