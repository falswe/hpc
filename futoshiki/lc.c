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

void compute_pc_lists(Futoshiki* puzzle) {
    int n = puzzle->size;

    for (int row = 0; row < n; row++) {
        for (int col = 0; col < n; col++) {
            puzzle->pc_lengths[row][col] = 0;

            // For given cells, only the given value is possible
            if (puzzle->board[row][col] != EMPTY) {
                puzzle->pc_list[row][col][0] = puzzle->board[row][col];
                puzzle->pc_lengths[row][col] = 1;
                continue;
            }

            // For empty cells, try all values and check constraints
            for (int c = 1; c <= n; c++) {
                bool valid = true;

                // Check vertical constraints for cell ABOVE current cell
                if (row > 0) {
                    // If cell above has '>' constraint, current cell can't be n
                    // because nothing could be greater than n
                    if (puzzle->v_cons[row - 1][col] == GREATER && c >= n) {
                        valid = false;
                    }
                    // If cell above has '<' constraint, current cell can't be 1
                    // because nothing could be smaller than 1
                    if (puzzle->v_cons[row - 1][col] == SMALLER && c <= 1) {
                        valid = false;
                    }
                // Check vertical constraints for cell BELOW current cell
                } else if (row < n - 1) {
                    // If current cell has '>' constraint, it can't be 1
                    // because nothing could be smaller than 1
                    if (puzzle->v_cons[row][col] == GREATER && c <= 1) {
                        valid = false;
                    }
                    // If current cell has '<' constraint, it can't be n
                    // because nothing could be greater than n
                    if (puzzle->v_cons[row][col] == SMALLER && c >= n) {
                        valid = false;
                    }
                }

                // Check horizontal constraints for cell LEFT of current cell
                if (col > 0) {
                    // If cell to left has '>' constraint, current cell can't be n
                    // because nothing could be greater than n
                    if (puzzle->h_cons[row][col - 1] == GREATER && c >= n) {
                        valid = false;
                    }
                    // If cell to left has '<' constraint, current cell can't be 1
                    // because nothing could be smaller than 1
                    if (puzzle->h_cons[row][col - 1] == SMALLER && c <= 1) {
                        valid = false;
                    }
                // Check horizontal constraints for cell RIGHT of current cell
                } else if (col < n - 1) {
                    // If current cell has '>' constraint, it can't be 1
                    // because nothing could be smaller than 1
                    if (puzzle->h_cons[row][col] == GREATER && c <= 1) {
                        valid = false;
                    }
                    // If current cell has '<' constraint, it can't be n
                    // because nothing could be greater than n
                    if (puzzle->h_cons[row][col] == SMALLER && c >= n) {
                        valid = false;
                    }
                }

                // If value passes inequality constraints and general safety check
                // (row/column uniqueness), add it to possible values
                if (valid && safe(puzzle, row, col, puzzle->board, c)) {
                    puzzle->pc_list[row][col][puzzle->pc_lengths[row][col]++] = c;
                }
            }
        }
    }
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