#include "futoshiki.h"

#include <ctype.h>
#include <math.h>
#include <mpi.h>
#include <omp.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>

#include "comparison.h"

// Original safe function with OpenMP parallelism
bool safe(const Futoshiki* puzzle, int row, int col, int solution[MAX_N][MAX_N], int color) {
    // If cell has a given color, only allow that color
    if (puzzle->board[row][col] != EMPTY) {
        return puzzle->board[row][col] == color;
    }

    // Check horizontal inequality constraints
    if (col > 0) {
        if (puzzle->h_cons[row][col - 1] == GREATER &&  //
            solution[row][col - 1] != EMPTY &&          //
            solution[row][col - 1] <= color) {
            return false;
        }
        if (puzzle->h_cons[row][col - 1] == SMALLER &&  //
            solution[row][col - 1] != EMPTY &&          //
            solution[row][col - 1] >= color) {
            return false;
        }
    }
    if (col < puzzle->size - 1) {
        if (puzzle->h_cons[row][col] == GREATER &&  //
            solution[row][col + 1] != EMPTY &&      //
            color <= solution[row][col + 1]) {
            return false;
        }
        if (puzzle->h_cons[row][col] == SMALLER &&  //
            solution[row][col + 1] != EMPTY &&      //
            color >= solution[row][col + 1]) {
            return false;
        }
    }

    // Check vertical inequality constraints
    if (row > 0) {
        if (puzzle->v_cons[row - 1][col] == GREATER &&  //
            solution[row - 1][col] != EMPTY &&          //
            solution[row - 1][col] <= color) {
            return false;
        }
        if (puzzle->v_cons[row - 1][col] == SMALLER &&  //
            solution[row - 1][col] != EMPTY &&          //
            solution[row - 1][col] >= color) {
            return false;
        }
    }
    if (row < puzzle->size - 1) {
        if (puzzle->v_cons[row][col] == GREATER &&  //
            solution[row + 1][col] != EMPTY &&      //
            color <= solution[row + 1][col]) {
            return false;
        }
        if (puzzle->v_cons[row][col] == SMALLER &&  //
            solution[row + 1][col] != EMPTY &&      //
            color >= solution[row + 1][col]) {
            return false;
        }
    }

    // Check for duplicates in row and column - can be parallelized
    bool duplicate_found = false;

#pragma omp parallel for reduction(|| : duplicate_found)
    for (int i = 0; i < puzzle->size; i++) {
        if (i != col && solution[row][i] == color) {
            duplicate_found = true;
        }
        if (i != row && solution[i][col] == color) {
            duplicate_found = true;
        }
    }

    if (duplicate_found) {
        return false;
    }

    return true;
}

bool has_valid_neighbor(const Futoshiki* puzzle, int row, int col, int color, bool need_greater) {
    for (int i = 0; i < puzzle->pc_lengths[row][col]; i++) {
        int neighbor_color = puzzle->pc_list[row][col][i];
        if ((need_greater && neighbor_color > color) ||  //
            (!need_greater && neighbor_color < color)) {
            return true;
        }
    }
    return false;
}

bool satisfies_inequalities(const Futoshiki* puzzle, int row, int col, int color) {
    // Check horizontal constraints
    if (col > 0) {
        switch (puzzle->h_cons[row][col - 1]) {
            case GREATER:  // Left > Current
                if (!has_valid_neighbor(puzzle, row, col - 1, color, true)) {
                    return false;
                }
                break;
            case SMALLER:  // Left < Current
                if (!has_valid_neighbor(puzzle, row, col - 1, color, false)) {
                    return false;
                }
                break;
        }
    }

    if (col < puzzle->size - 1) {
        switch (puzzle->h_cons[row][col]) {
            case GREATER:  // Current > Right
                if (!has_valid_neighbor(puzzle, row, col + 1, color, false)) {
                    return false;
                }
                break;
            case SMALLER:  // Current < Right
                if (!has_valid_neighbor(puzzle, row, col + 1, color, true)) {
                    return false;
                }
                break;
        }
    }

    // Check vertical constraints
    if (row > 0) {
        switch (puzzle->v_cons[row - 1][col]) {
            case GREATER:  // Upper > Current
                if (!has_valid_neighbor(puzzle, row - 1, col, color, true)) {
                    return false;
                }
                break;
            case SMALLER:  // Upper < Current
                if (!has_valid_neighbor(puzzle, row - 1, col, color, false)) {
                    return false;
                }
                break;
        }
    }

    if (row < puzzle->size - 1) {
        switch (puzzle->v_cons[row][col]) {
            case GREATER:  // Current > Lower
                if (!has_valid_neighbor(puzzle, row + 1, col, color, false)) {
                    return false;
                }
                break;
            case SMALLER:  // Current < Lower
                if (!has_valid_neighbor(puzzle, row + 1, col, color, true)) {
                    return false;
                }
                break;
        }
    }

    return true;
}

// Thread-safe version of filter_possible_colors
void filter_possible_colors(Futoshiki* puzzle, int row, int col) {
    if (puzzle->board[row][col] != EMPTY) {
        puzzle->pc_lengths[row][col] = 1;
        puzzle->pc_list[row][col][0] = puzzle->board[row][col];
        return;
    }

    int new_length = 0;
    int temp_list[MAX_N];

    // Check each color to see if it satisfies inequalities
    for (int i = 0; i < puzzle->pc_lengths[row][col]; i++) {
        int color = puzzle->pc_list[row][col][i];
        if (satisfies_inequalities(puzzle, row, col, color)) {
            temp_list[new_length++] = color;
        }
    }

    // Update the pc_list with valid colors
    for (int i = 0; i < new_length; i++) {
        puzzle->pc_list[row][col][i] = temp_list[i];
    }
    puzzle->pc_lengths[row][col] = new_length;
}

void process_uniqueness(Futoshiki* puzzle, int row, int col) {
    if (puzzle->pc_lengths[row][col] == 1) {
        int color = puzzle->pc_list[row][col][0];
        for (int i = 0; i < puzzle->size; i++) {
            if (i != col) {  // Remove from row
                int new_length = 0;
                for (int j = 0; j < puzzle->pc_lengths[row][i]; j++) {
                    if (puzzle->pc_list[row][i][j] != color) {
                        puzzle->pc_list[row][i][new_length++] = puzzle->pc_list[row][i][j];
                    }
                }
                puzzle->pc_lengths[row][i] = new_length;
            }
            if (i != row) {  // Remove from column
                int new_length = 0;
                for (int j = 0; j < puzzle->pc_lengths[i][col]; j++) {
                    if (puzzle->pc_list[i][col][j] != color) {
                        puzzle->pc_list[i][col][new_length++] = puzzle->pc_list[i][col][j];
                    }
                }
                puzzle->pc_lengths[i][col] = new_length;
            }
        }
    }
}

// Parallel version of compute_pc_lists with OpenMP
int compute_pc_lists(Futoshiki* puzzle, bool use_precoloring) {
    int total_colors_removed = 0;
    int initial_colors = 0;

// Initialize pc_lists - can be parallelized with OpenMP
#pragma omp parallel for collapse(2) reduction(+ : initial_colors)
    for (int row = 0; row < puzzle->size; row++) {
        for (int col = 0; col < puzzle->size; col++) {
            puzzle->pc_lengths[row][col] = 0;

            // Consider pre-set colors of the board
            if (puzzle->board[row][col] != EMPTY) {
                puzzle->pc_list[row][col][0] = puzzle->board[row][col];
                puzzle->pc_lengths[row][col] = 1;
                initial_colors += 1;  // Only count 1 since it's preset
                continue;
            }

            // Initialize with all possible colors
            for (int color = 1; color <= puzzle->size; color++) {
                puzzle->pc_list[row][col][puzzle->pc_lengths[row][col]++] = color;
            }
            initial_colors += puzzle->size;
        }
    }

    if (use_precoloring) {
        bool changes;
        int loop_colors_removed = 0;

        do {
            changes = false;
            int old_lengths[MAX_N][MAX_N];
            memcpy(old_lengths, puzzle->pc_lengths, sizeof(old_lengths));
            loop_colors_removed = 0;

// Process each cell - can be parallelized with OpenMP
#pragma omp parallel
            {
                int thread_colors_removed = 0;
                bool thread_changes = false;

#pragma omp for collapse(2)
                for (int row = 0; row < puzzle->size; row++) {
                    for (int col = 0; col < puzzle->size; col++) {
                        int before_length = puzzle->pc_lengths[row][col];

// These functions need thread safety
#pragma omp critical
                        {
                            filter_possible_colors(puzzle, row, col);
                            process_uniqueness(puzzle, row, col);
                        }

                        int after_length = puzzle->pc_lengths[row][col];
                        thread_colors_removed += (before_length - after_length);

                        if (after_length != old_lengths[row][col]) {
                            thread_changes = true;
                        }
                    }
                }

// Combine results from all threads
#pragma omp critical
                {
                    loop_colors_removed += thread_colors_removed;
                    changes |= thread_changes;
                }
            }

            total_colors_removed += loop_colors_removed;

        } while (changes);
    }

    return total_colors_removed;
}

bool color_g(Futoshiki* puzzle, int solution[MAX_N][MAX_N], int row, int col) {
    if (row >= puzzle->size) {
        return true;
    }

    if (col >= puzzle->size) {
        return color_g(puzzle, solution, row + 1, 0);
    }

    // Skip given cells
    if (puzzle->board[row][col] != EMPTY) {
        solution[row][col] = puzzle->board[row][col];
        return color_g(puzzle, solution, row, col + 1);
    }

    // Try each possible color for current cell
    for (int i = 0; i < puzzle->pc_lengths[row][col]; i++) {
        int color = puzzle->pc_list[row][col][i];
        if (safe(puzzle, row, col, solution, color)) {
            solution[row][col] = color;
            if (color_g(puzzle, solution, row, col + 1)) {
                return true;
            }
            solution[row][col] = EMPTY;
        }
    }

    return false;
}

void print_board(const Futoshiki* puzzle, int solution[MAX_N][MAX_N]) {
    for (int row = 0; row < puzzle->size; row++) {
        for (int col = 0; col < puzzle->size; col++) {
            printf(" %d ", solution[row][col]);
            if (col < puzzle->size - 1) {
                switch (puzzle->h_cons[row][col]) {
                    case GREATER:
                        printf(">");
                        break;
                    case SMALLER:
                        printf("<");
                        break;
                    default:
                        printf(" ");
                        break;
                }
            }
        }
        printf("\n");
        if (row < puzzle->size - 1) {
            for (int col = 0; col < puzzle->size; col++) {
                switch (puzzle->v_cons[row][col]) {
                    case GREATER:
                        printf(" v ");
                        break;
                    case SMALLER:
                        printf(" ^ ");
                        break;
                    default:
                        printf("   ");
                        break;
                }
                if (col < puzzle->size - 1) printf(" ");
            }
            printf("\n");
        }
    }
    printf("\n");
}

bool parse_futoshiki(const char* input, Futoshiki* puzzle) {
    // Initialize everything to 0/NO_CONS
    memset(puzzle->board, 0, sizeof(puzzle->board));
    memset(puzzle->h_cons, NO_CONS, sizeof(puzzle->h_cons));
    memset(puzzle->v_cons, NO_CONS, sizeof(puzzle->v_cons));

    // First, determine size by counting numbers in first row
    char first_line[256];
    int line_len = 0;
    while (input[line_len] && input[line_len] != '\n') {
        first_line[line_len] = input[line_len];
        line_len++;
    }
    first_line[line_len] = '\0';

    // Count numbers in first line to determine size
    puzzle->size = 0;
    for (int i = 0; i < line_len; i++) {
        if (isdigit(first_line[i]) || first_line[i] == '0') {
            puzzle->size++;
        }
    }

    if (puzzle->size > MAX_N || puzzle->size == 0) {
        return false;
    }

    char line[256];
    const char* line_start = input;
    int number_row = 0;

    while (*line_start) {
        // Copy line to buffer
        line_len = 0;
        while (line_start[line_len] && line_start[line_len] != '\n') {
            line[line_len] = line_start[line_len];
            line_len++;
        }
        line[line_len] = '\0';

        // Skip empty lines
        if (line_len == 0 || line[0] == '\n') {
            line_start += (line_start[0] == '\n' ? 1 : 0);
            continue;
        }

        // Check if this is a constraint line
        bool is_v_constraint_line = false;
        for (int i = 0; i < line_len; i++) {
            if (line[i] == '^' || line[i] == 'v' || line[i] == 'V') {
                is_v_constraint_line = true;
                break;
            }
        }

        if (!is_v_constraint_line) {  // Number and horizontal constraint line
            int col = 0;
            for (int i = 0; i < line_len && col < puzzle->size; i++) {
                if (line[i] == ' ') continue;

                if (isdigit(line[i])) {
                    puzzle->board[number_row][col] = line[i] - '0';
                    col++;
                } else if (line[i] == '<' && col > 0) {
                    puzzle->h_cons[number_row][col - 1] = SMALLER;
                } else if (line[i] == '>' && col > 0) {
                    puzzle->h_cons[number_row][col - 1] = GREATER;
                }
            }
            number_row++;
        } else {  // Vertical constraint line
            // Create a mapping array for character positions to grid columns
            int col_positions[MAX_N] = {0};  // Position where each column's number would be
            int pos = 0;
            for (int col = 0; col < puzzle->size; col++) {
                col_positions[col] = pos;
                pos += (col == puzzle->size - 1) ? 0 : 4;  // 4 spaces between numbers
            }

            // Now scan the constraint line
            for (int i = 0; i < line_len; i++) {
                if (line[i] != '^' && line[i] != 'v' && line[i] != 'V') continue;

                // Find which column this constraint is closest to
                int col = 0;
                for (int j = 1; j < puzzle->size; j++) {
                    if (abs(i - col_positions[j]) < abs(i - col_positions[col])) {
                        col = j;
                    }
                }

                if (col < puzzle->size) {
                    puzzle->v_cons[number_row - 1][col] = (line[i] == '^') ? SMALLER : GREATER;
                }
            }
        }

        line_start += line_len + (line_start[line_len] == '\n' ? 1 : 0);
    }

    return true;
}

// File reading function
bool read_puzzle_from_file(const char* filename, Futoshiki* puzzle) {
    FILE* file = fopen(filename, "r");
    if (!file) {
        printf("Error: Could not open file %s\n", filename);
        return false;
    }

    char buffer[1024] = {0};
    char content[1024] = {0};
    int total_read = 0;

    while (fgets(buffer, sizeof(buffer), file)) {
        strcat(content, buffer);
        total_read += strlen(buffer);
        if (total_read >= sizeof(content) - 1) {
            printf("Error: Puzzle file too large\n");
            fclose(file);
            return false;
        }
    }

    fclose(file);
    return parse_futoshiki(content, puzzle);
}

double get_time() {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_sec + tv.tv_usec * 1e-6;
}

// Original solve_puzzle function
SolverStats solve_puzzle(const char* filename, bool use_precoloring, bool print_details) {
    SolverStats stats = {0};
    Futoshiki puzzle;

    if (read_puzzle_from_file(filename, &puzzle)) {
        if (print_details) {
            printf("Initial puzzle:\n");
            int initial_board[MAX_N][MAX_N];
            memcpy(initial_board, puzzle.board, sizeof(initial_board));
            print_board(&puzzle, initial_board);
        }

        // Time the pre-coloring phase
        double start_precolor = get_time();
        stats.colors_removed = compute_pc_lists(&puzzle, use_precoloring);
        double end_precolor = get_time();
        stats.precolor_time = end_precolor - start_precolor;

        if (print_details) {
            printf("\nPossible colors for each cell:\n");
            for (int row = 0; row < puzzle.size; row++) {
                for (int col = 0; col < puzzle.size; col++) {
                    printf("Cell [%d][%d]: ", row, col);
                    for (int i = 0; i < puzzle.pc_lengths[row][col]; i++) {
                        printf("%d ", puzzle.pc_list[row][col][i]);
                    }
                    printf("\n");
                }
            }
        }

        // Time the list-coloring phase
        int solution[MAX_N][MAX_N] = {0};
        double start_coloring = get_time();
        stats.found_solution = color_g(&puzzle, solution, 0, 0);
        double end_coloring = get_time();
        stats.coloring_time = end_coloring - start_coloring;
        stats.total_time = stats.precolor_time + stats.coloring_time;

        // Calculate remaining colors and total processed
        stats.remaining_colors = 0;
        for (int row = 0; row < puzzle.size; row++) {
            for (int col = 0; col < puzzle.size; col++) {
                stats.remaining_colors += puzzle.pc_lengths[row][col];
            }
        }
        stats.total_processed = puzzle.size * puzzle.size * puzzle.size;

        if (print_details && stats.found_solution) {
            printf("\nSolution:\n");
            print_board(&puzzle, solution);
        }
    }

    return stats;
}

// Broadcast puzzle data to all processes
void broadcast_puzzle(Futoshiki* puzzle, int root) {
    // Broadcast puzzle size
    MPI_Bcast(&puzzle->size, 1, MPI_INT, root, MPI_COMM_WORLD);

    // Broadcast board data
    MPI_Bcast(&puzzle->board[0][0], MAX_N * MAX_N, MPI_INT, root, MPI_COMM_WORLD);

    // Broadcast horizontal constraints
    MPI_Bcast(&puzzle->h_cons[0][0], MAX_N * (MAX_N - 1), MPI_INT, root, MPI_COMM_WORLD);

    // Broadcast vertical constraints
    MPI_Bcast(&puzzle->v_cons[0][0], (MAX_N - 1) * MAX_N, MPI_INT, root, MPI_COMM_WORLD);
}

// Gather solution from processes
void gather_solution(int solution[MAX_N][MAX_N], int puzzle_size, bool* found_solution,
                     int* found_by) {
    int local_found = *found_solution ? 1 : 0;
    int global_found = 0;

    // Find out if any process found a solution
    MPI_Allreduce(&local_found, &global_found, 1, MPI_INT, MPI_MAX, MPI_COMM_WORLD);
    *found_solution = (global_found > 0);

    // If a solution was found, broadcast it from the process that found it
    if (*found_solution) {
        int local_rank = *found_by;
        MPI_Allreduce(&local_rank, found_by, 1, MPI_INT, MPI_MIN, MPI_COMM_WORLD);
        MPI_Bcast(&solution[0][0], MAX_N * MAX_N, MPI_INT, *found_by, MPI_COMM_WORLD);
    }
}

// Parallel version of solve_puzzle
SolverStats solve_puzzle_parallel(const char* filename, bool use_precoloring, bool print_details,
                                  int rank, int size) {
    SolverStats stats = {0};
    Futoshiki puzzle;
    bool read_success = true;

    // Root process reads the puzzle file
    if (rank == 0) {
        read_success = read_puzzle_from_file(filename, &puzzle);
        if (read_success && print_details) {
            printf("Initial puzzle:\n");
            int initial_board[MAX_N][MAX_N];
            memcpy(initial_board, puzzle.board, sizeof(initial_board));
            print_board(&puzzle, initial_board);
        }
    }

    // Broadcast read success status
    MPI_Bcast(&read_success, 1, MPI_C_BOOL, 0, MPI_COMM_WORLD);
    if (!read_success) {
        return stats;
    }

    // Broadcast puzzle data to all processes
    broadcast_puzzle(&puzzle, 0);

    // Time the pre-coloring phase
    double start_precolor = get_time();

    // Compute possible colors for each cell
    stats.colors_removed = compute_pc_lists(&puzzle, use_precoloring);

    double end_precolor = get_time();
    stats.precolor_time = end_precolor - start_precolor;

    // Print possible colors (only root process)
    if (rank == 0 && print_details) {
        printf("\nPossible colors for each cell:\n");
        for (int row = 0; row < puzzle.size; row++) {
            for (int col = 0; col < puzzle.size; col++) {
                printf("Cell [%d][%d]: ", row, col);
                for (int i = 0; i < puzzle.pc_lengths[row][col]; i++) {
                    printf("%d ", puzzle.pc_list[row][col][i]);
                }
                printf("\n");
            }
        }
    }

    // Initialize solution matrix
    int solution[MAX_N][MAX_N] = {0};
    bool found_solution = false;
    int found_by = -1;

    // Time the list-coloring phase
    double start_coloring = get_time();

    // Find first empty cell to start distribution
    int start_row = 0, start_col = 0;
    bool found_empty = false;

    for (int r = 0; r < puzzle.size && !found_empty; r++) {
        for (int c = 0; c < puzzle.size && !found_empty; c++) {
            if (puzzle.board[r][c] == EMPTY) {
                start_row = r;
                start_col = c;
                found_empty = true;
            }
        }
    }

    if (found_empty) {
        // Distribute work based on possible colors for the first empty cell
        for (int i = rank; i < puzzle.pc_lengths[start_row][start_col]; i += size) {
            int color = puzzle.pc_list[start_row][start_col][i];
            if (safe(&puzzle, start_row, start_col, solution, color)) {
                solution[start_row][start_col] = color;
                if (color_g(&puzzle, solution, start_row, start_col + 1)) {
                    found_solution = true;
                    found_by = rank;
                    break;
                }
                solution[start_row][start_col] = EMPTY;
            }
        }
    } else {
        // No empty cells (puzzle is already solved)
        found_solution = true;
        if (rank == 0) {
            for (int r = 0; r < puzzle.size; r++) {
                for (int c = 0; c < puzzle.size; c++) {
                    solution[r][c] = puzzle.board[r][c];
                }
            }
            found_by = 0;
        }
    }

    // Gather solution from all processes
    gather_solution(solution, puzzle.size, &found_solution, &found_by);

    double end_coloring = get_time();
    stats.coloring_time = end_coloring - start_coloring;
    stats.total_time = stats.precolor_time + stats.coloring_time;
    stats.found_solution = found_solution;

    // Calculate remaining colors and total processed
    stats.remaining_colors = 0;
    for (int row = 0; row < puzzle.size; row++) {
        for (int col = 0; col < puzzle.size; col++) {
            stats.remaining_colors += puzzle.pc_lengths[row][col];
        }
    }
    stats.total_processed = puzzle.size * puzzle.size * puzzle.size;

    // Only root process prints the solution
    if (rank == 0 && print_details && stats.found_solution) {
        printf("\nSolution:\n");
        print_board(&puzzle, solution);
    }

    return stats;
}