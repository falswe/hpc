#include <stdio.h>
#include <string.h>

#include "comparison.h"
#include "futoshiki.h"

int main(int argc, char* argv[]) {
    if (argc != 2 && argc != 3) {
        printf("Usage: %s <puzzle_file> [-c|-n]\n", argv[0]);
        printf("  -c: comparison mode (run both with and without precoloring)\n");
        printf("  -n: disable precoloring\n");
        return 1;
    }

    if (argc == 3 && strcmp(argv[2], "-c") == 0) {
        run_comparison(argv[1]);
    } else {
        bool use_precoloring = (argc == 2 || strcmp(argv[2], "-n") != 0);
        SolverStats stats = solve_puzzle(argv[1], use_precoloring, true);
        print_stats(&stats, "");
    }

    return 0;
}