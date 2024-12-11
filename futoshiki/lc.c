/*
 * This code implements a solver for the Futoshiki puzzle game.
 * Futoshiki is a puzzle where you fill an NxN grid with numbers 1 to N,
 * ensuring each number appears exactly once in each row and column while
 * satisfying inequality constraints between adjacent cells.
 */

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_N 50
#define EMPTY 0

typedef enum { NO_CONS = 0, GREATER = 1, SMALLER = 2 } Constraint;

typedef struct {
    int size;                 // Size of the puzzle (N)
    int board[MAX_N][MAX_N];  // The puzzle grid (0 means empty cell)
    Constraint h_cons[MAX_N][MAX_N - 1];  // Horizontal inequality constraints
    Constraint v_cons[MAX_N - 1][MAX_N];  // Vertical inequality constraints
    int pc_list[MAX_N][MAX_N][MAX_N];     // Possible colors for each cell
                                          // [row][col][possible_values]
    int pc_lengths[MAX_N][MAX_N];  // Possible colors list length for each cell
} Futoshiki;

bool safe(const Futoshiki* puzzle, int row, int col, int solution[MAX_N][MAX_N],
          int c) {
    // If cell has a given value, only allow that value
    if (puzzle->board[row][col] != EMPTY) {
        return puzzle->board[row][col] == c;
    }

    // Check horizontal inequality constraints
    if (col > 0) {
        if (puzzle->h_cons[row][col - 1] == GREATER &&
            solution[row][col - 1] != EMPTY && solution[row][col - 1] <= c) {
            return false;
        }
        if (puzzle->h_cons[row][col - 1] == SMALLER &&
            solution[row][col - 1] != EMPTY && solution[row][col - 1] >= c) {
            return false;
        }
    }
    if (col < puzzle->size - 1) {
        if (puzzle->h_cons[row][col] == GREATER &&
            solution[row][col + 1] != EMPTY && c <= solution[row][col + 1]) {
            return false;
        }
        if (puzzle->h_cons[row][col] == SMALLER &&
            solution[row][col + 1] != EMPTY && c >= solution[row][col + 1]) {
            return false;
        }
    }

    // Check vertical inequality constraints
    if (row > 0) {
        if (puzzle->v_cons[row - 1][col] == GREATER &&
            solution[row - 1][col] != EMPTY && solution[row - 1][col] <= c) {
            return false;
        }
        if (puzzle->v_cons[row - 1][col] == SMALLER &&
            solution[row - 1][col] != EMPTY && solution[row - 1][col] >= c) {
            return false;
        }
    }
    if (row < puzzle->size - 1) {
        if (puzzle->v_cons[row][col] == GREATER &&
            solution[row + 1][col] != EMPTY && c <= solution[row + 1][col]) {
            return false;
        }
        if (puzzle->v_cons[row][col] == SMALLER &&
            solution[row + 1][col] != EMPTY && c >= solution[row + 1][col]) {
            return false;
        }
    }

    // Check for duplicates in row and column
    for (int i = 0; i < puzzle->size; i++) {
        if (i != col && solution[row][i] == c) return false;
        if (i != row && solution[i][col] == c) return false;
    }

    return true;
}
int get_min_value(const Futoshiki* puzzle, int row, int col) {
    return puzzle->pc_list[row][col][0];  // List is always sorted
}

int get_max_value(const Futoshiki* puzzle, int row, int col) {
    return puzzle->pc_list[row][col][puzzle->pc_lengths[row][col] - 1];
}

void remove_forbidden_values(Futoshiki* puzzle, int row, int col,
                             int forbidden_start, int forbidden_end) {
    for (int i = 0; i < puzzle->pc_lengths[row][col]; i++) {
        int val = puzzle->pc_list[row][col][i];
        if (val >= forbidden_start && val <= forbidden_end) {
            // Remove this value by shifting remaining elements
            for (int j = i; j < puzzle->pc_lengths[row][col] - 1; j++) {
                puzzle->pc_list[row][col][j] = puzzle->pc_list[row][col][j + 1];
            }
            puzzle->pc_lengths[row][col]--;
            i--;  // Recheck current position as we shifted elements
        }
    }
}

void process_inequality_chain(Futoshiki* puzzle, int row, int col, int dir_row,
                              int dir_col, bool check_smaller) {
    int chain_length = 0;
    int curr_row = row;
    int curr_col = col;

    // First, find length of chain and store cells in the chain
    struct {
        int row;
        int col;
    } chain_cells[MAX_N];  // Can't be longer than puzzle size
    chain_cells[0].row = row;
    chain_cells[0].col = col;

    while (true) {
        int next_row = curr_row + dir_row;
        int next_col = curr_col + dir_col;

        // Check if we're still in bounds
        if (next_row < 0 || next_row >= puzzle->size || next_col < 0 ||
            next_col >= puzzle->size) {
            break;
        }

        // Check if constraint exists and matches our direction
        bool has_constraint = false;
        if (dir_row == 0) {  // Horizontal chain
            has_constraint = (check_smaller &&
                              puzzle->h_cons[curr_row][curr_col] == SMALLER) ||
                             (!check_smaller &&
                              puzzle->h_cons[curr_row][curr_col] == GREATER);
        } else {  // Vertical chain
            has_constraint = (check_smaller &&
                              puzzle->v_cons[curr_row][curr_col] == SMALLER) ||
                             (!check_smaller &&
                              puzzle->v_cons[curr_row][curr_col] == GREATER);
        }

        if (!has_constraint) {
            break;
        }

        chain_length++;
        curr_row = next_row;
        curr_col = next_col;
        chain_cells[chain_length].row = curr_row;
        chain_cells[chain_length].col = curr_col;
    }

    // Process each cell in the chain
    for (int pos = 0; pos <= chain_length; pos++) {
        int curr_row = chain_cells[pos].row;
        int curr_col = chain_cells[pos].col;

        if (check_smaller) {  // Chain of < constraints
            if (pos < chain_length) {
                // Current cell must be smaller than next cell's maximum
                int next_row = chain_cells[pos + 1].row;
                int next_col = chain_cells[pos + 1].col;
                int next_max = get_max_value(puzzle, next_row, next_col);
                remove_forbidden_values(puzzle, curr_row, curr_col, next_max,
                                        puzzle->size);
            }
            if (pos > 0) {
                // Current cell must be larger than previous cell's minimum
                int prev_row = chain_cells[pos - 1].row;
                int prev_col = chain_cells[pos - 1].col;
                int prev_min = get_min_value(puzzle, prev_row, prev_col);
                remove_forbidden_values(puzzle, curr_row, curr_col, 1,
                                        prev_min);
            }
        } else {  // Chain of > constraints
            if (pos < chain_length) {
                // Current cell must be larger than next cell's minimum
                int next_row = chain_cells[pos + 1].row;
                int next_col = chain_cells[pos + 1].col;
                int next_min = get_min_value(puzzle, next_row, next_col);
                remove_forbidden_values(puzzle, curr_row, curr_col, 1,
                                        next_min);
            }
            if (pos > 0) {
                // Current cell must be smaller than previous cell's maximum
                int prev_row = chain_cells[pos - 1].row;
                int prev_col = chain_cells[pos - 1].col;
                int prev_max = get_max_value(puzzle, prev_row, prev_col);
                remove_forbidden_values(puzzle, curr_row, curr_col, prev_max,
                                        puzzle->size);
            }
        }
    }
}

// Helper function to compare two pc_lists
bool pc_lists_equal(const Futoshiki* puzzle,
                    const int old_pc_list[MAX_N][MAX_N][MAX_N],
                    const int old_lengths[MAX_N][MAX_N]) {
    for (int row = 0; row < puzzle->size; row++) {
        for (int col = 0; col < puzzle->size; col++) {
            if (puzzle->pc_lengths[row][col] != old_lengths[row][col]) {
                return false;
            }
            for (int i = 0; i < puzzle->pc_lengths[row][col]; i++) {
                if (puzzle->pc_list[row][col][i] != old_pc_list[row][col][i]) {
                    return false;
                }
            }
        }
    }
    return true;
}

void compute_pc_lists(Futoshiki* puzzle) {
    int n = puzzle->size;

    // Initialize pc_lists
    for (int row = 0; row < n; row++) {
        for (int col = 0; col < n; col++) {
            puzzle->pc_lengths[row][col] = 0;

            if (puzzle->board[row][col] != EMPTY) {
                puzzle->pc_list[row][col][0] = puzzle->board[row][col];
                puzzle->pc_lengths[row][col] = 1;
                continue;
            }

            // Initialize with all possible values
            for (int c = 1; c <= n; c++) {
                puzzle->pc_list[row][col][puzzle->pc_lengths[row][col]++] = c;
            }
        }
    }

    // Keep iterating until no changes are made
    bool changes_made;
    do {
        changes_made = false;

        // Store old pc_lists to detect changes
        int old_pc_list[MAX_N][MAX_N][MAX_N];
        int old_lengths[MAX_N][MAX_N];
        memcpy(old_pc_list, puzzle->pc_list, sizeof(old_pc_list));
        memcpy(old_lengths, puzzle->pc_lengths, sizeof(old_lengths));

        // Process all constraints as chains
        for (int row = 0; row < n; row++) {
            for (int col = 0; col < n; col++) {
                process_inequality_chain(puzzle, row, col, 0, 1,
                                         true);  // horizontal ascending
                process_inequality_chain(puzzle, row, col, 0, 1,
                                         false);  // horizontal descending
                process_inequality_chain(puzzle, row, col, 1, 0,
                                         true);  // vertical ascending
                process_inequality_chain(puzzle, row, col, 1, 0,
                                         false);  // vertical descending
            }
        }

        // Create temporary solution matrix for checking
        int temp_solution[MAX_N][MAX_N] = {0};

        // Copy fixed values and guaranteed minimum values for checking
        for (int row = 0; row < n; row++) {
            for (int col = 0; col < n; col++) {
                if (puzzle->pc_lengths[row][col] == 1) {
                    temp_solution[row][col] = puzzle->pc_list[row][col][0];
                }
            }
        }

        // Filter remaining possible values based on fixed values and
        // constraints
        for (int row = 0; row < n; row++) {
            for (int col = 0; col < n; col++) {
                if (puzzle->pc_lengths[row][col] > 1) {
                    int new_length = 0;
                    for (int i = 0; i < puzzle->pc_lengths[row][col]; i++) {
                        int c = puzzle->pc_list[row][col][i];
                        if (safe(puzzle, row, col, temp_solution, c)) {
                            puzzle->pc_list[row][col][new_length++] = c;
                        }
                    }
                    puzzle->pc_lengths[row][col] = new_length;
                }
            }
        }

        // Check if any changes were made
        if (!pc_lists_equal(puzzle, old_pc_list, old_lengths)) {
            changes_made = true;
        }
    } while (changes_made);
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

    // Try each possible value for current cell
    for (int i = 0; i < puzzle->pc_lengths[row][col]; i++) {
        int c = puzzle->pc_list[row][col][i];
        if (safe(puzzle, row, col, solution, c)) {
            solution[row][col] = c;
            if (color_g(puzzle, solution, row, col + 1)) {
                return true;
            }
            solution[row][col] = EMPTY;
        }
    }

    return false;
}

void print_board(const Futoshiki* puzzle, int solution[MAX_N][MAX_N]) {
    for (int i = 0; i < puzzle->size; i++) {
        for (int j = 0; j < puzzle->size; j++) {
            printf(" %d ", solution[i][j]);
            if (j < puzzle->size - 1) {
                switch (puzzle->h_cons[i][j]) {
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
        if (i < puzzle->size - 1) {
            for (int j = 0; j < puzzle->size; j++) {
                switch (puzzle->v_cons[i][j]) {
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
                if (j < puzzle->size - 1) printf(" ");
            }
            printf("\n");
        }
    }
    printf("\n");
}

int main() {
    // Example puzzle initialization
    Futoshiki puzzle = {.size = 4,
                        .board = {{0, 0, 0, 0},  //
                                  {0, 0, 0, 0},
                                  {0, 0, 0, 0},
                                  {0, 0, 0, 3}},
                        .h_cons = {{NO_CONS, NO_CONS, NO_CONS},
                                   {NO_CONS, NO_CONS, NO_CONS},
                                   {NO_CONS, SMALLER, SMALLER},
                                   {NO_CONS, NO_CONS, NO_CONS}},
                        .v_cons = {{GREATER, NO_CONS, GREATER, NO_CONS},
                                   {NO_CONS, NO_CONS, NO_CONS, NO_CONS},
                                   {NO_CONS, GREATER, NO_CONS, NO_CONS}}};

    printf("Initial Puzzle:\n");
    int initial_board[MAX_N][MAX_N];
    memcpy(initial_board, puzzle.board, sizeof(initial_board));
    print_board(&puzzle, initial_board);

    // Compute possible values for each cell
    compute_pc_lists(&puzzle);

    // Print possible values for each cell
    printf("Possible values for each cell:\n");
    for (int row = 0; row < puzzle.size; row++) {
        for (int col = 0; col < puzzle.size; col++) {
            printf("Cell [%d][%d]: ", row, col);
            for (int i = 0; i < puzzle.pc_lengths[row][col]; i++) {
                printf("%d ", puzzle.pc_list[row][col][i]);
            }
            printf("\n");
        }
    }

    // Initialize solution array
    int solution[MAX_N][MAX_N] = {0};

    // Try to solve the puzzle
    if (color_g(&puzzle, solution, 0, 0)) {
        printf("\nSolution found:\n");
        print_board(&puzzle, solution);
    } else {
        printf("\nNo solution exists.\n");
    }

    return 0;
}