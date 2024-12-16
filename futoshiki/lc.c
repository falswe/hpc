#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_N 50
#define EMPTY 0

typedef enum { NO_CONS = 0, GREATER = 1, SMALLER = 2 } Constraint;

typedef struct {
    int size;                             // Size of the puzzle (N)
    int board[MAX_N][MAX_N];              // The puzzle grid (0 means empty cell)
    Constraint h_cons[MAX_N][MAX_N - 1];  // Horizontal inequality constraints
    Constraint v_cons[MAX_N - 1][MAX_N];  // Vertical inequality constraints
    int pc_list[MAX_N][MAX_N][MAX_N];     // Possible colors for each cell
                                          // [row][col][possible_values]
    int pc_lengths[MAX_N][MAX_N];         // Possible colors list length for each cell
} Futoshiki;

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

    // Check for duplicates in row and column
    for (int i = 0; i < puzzle->size; i++) {
        if (i != col && solution[row][i] == color) return false;
        if (i != row && solution[i][col] == color) return false;
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

void filter_possible_colors(Futoshiki* puzzle, int row, int col) {
    if (puzzle->board[row][col] != EMPTY) {
        puzzle->pc_lengths[row][col] = 1;
        puzzle->pc_list[row][col][0] = puzzle->board[row][col];
        return;
    }

    int new_length = 0;
    for (int i = 0; i < puzzle->pc_lengths[row][col]; i++) {
        int color = puzzle->pc_list[row][col][i];
        if (satisfies_inequalities(puzzle, row, col, color)) {
            puzzle->pc_list[row][col][new_length++] = color;
        }
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

void compute_pc_lists(Futoshiki* puzzle) {
    // Initialize pc_lists
    for (int row = 0; row < puzzle->size; row++) {
        for (int col = 0; col < puzzle->size; col++) {
            puzzle->pc_lengths[row][col] = 0;

            if (puzzle->board[row][col] != EMPTY) {
                puzzle->pc_list[row][col][0] = puzzle->board[row][col];
                puzzle->pc_lengths[row][col] = 1;
                continue;
            }

            // Initialize with all possible values
            for (int color = 1; color <= puzzle->size; color++) {
                puzzle->pc_list[row][col][puzzle->pc_lengths[row][col]++] = color;
            }
        }
    }

    bool changes;
    do {
        changes = false;
        int old_lengths[MAX_N][MAX_N];
        memcpy(old_lengths, puzzle->pc_lengths, sizeof(old_lengths));

        // Process each cell
        for (int row = 0; row < puzzle->size; row++) {
            for (int col = 0; col < puzzle->size; col++) {
                filter_possible_colors(puzzle, row, col);
                process_uniqueness(puzzle, row, col);
            }
        }

        // Check for changes
        for (int row = 0; row < puzzle->size; row++) {
            for (int col = 0; col < puzzle->size; col++) {
                if (puzzle->pc_lengths[row][col] != old_lengths[row][col]) {
                    changes = true;
                }
            }
        }
    } while (changes);
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

int main() {
    // From the paper
    // Futoshiki puzzle = {.size = 4,
    //                     .board = {{0, 0, 0, 0},  //
    //                               {0, 0, 0, 0},
    //                               {0, 0, 0, 0},
    //                               {0, 0, 0, 3}},
    //                     .h_cons = {{NO_CONS, NO_CONS, NO_CONS},
    //                                {NO_CONS, NO_CONS, NO_CONS},
    //                                {NO_CONS, SMALLER, SMALLER},
    //                                {NO_CONS, NO_CONS, NO_CONS}},
    //                     .v_cons = {{GREATER, NO_CONS, GREATER, NO_CONS},
    //                                {NO_CONS, NO_CONS, NO_CONS, NO_CONS},
    //                                {NO_CONS, GREATER, NO_CONS, NO_CONS}}};

    // Hard
    Futoshiki puzzle =  //
        {.size = 9,
         .board = {{0, 0, 0, 0, 0, 0, 0, 0, 0},  //
                   {0, 0, 0, 0, 5, 0, 0, 0, 0},
                   {0, 0, 0, 0, 0, 8, 7, 0, 0},
                   {0, 7, 0, 0, 0, 0, 2, 0, 0},
                   {0, 0, 0, 0, 0, 0, 0, 0, 0},
                   {0, 0, 0, 0, 0, 0, 0, 0, 0},
                   {7, 5, 0, 0, 0, 4, 0, 0, 0},
                   {0, 0, 0, 0, 0, 0, 0, 0, 6},
                   {0, 0, 0, 0, 0, 0, 0, 0, 0}},
         .h_cons =  //
         {{SMALLER, NO_CONS, GREATER, NO_CONS, NO_CONS, NO_CONS, SMALLER, NO_CONS},
          {NO_CONS, NO_CONS, NO_CONS, SMALLER, NO_CONS, SMALLER, NO_CONS, NO_CONS},
          {NO_CONS, NO_CONS, NO_CONS, SMALLER, NO_CONS, NO_CONS, NO_CONS, SMALLER},
          {NO_CONS, NO_CONS, SMALLER, SMALLER, NO_CONS, GREATER, NO_CONS, NO_CONS},
          {NO_CONS, GREATER, NO_CONS, NO_CONS, NO_CONS, SMALLER, NO_CONS, NO_CONS},
          {NO_CONS, NO_CONS, GREATER, NO_CONS, NO_CONS, NO_CONS, NO_CONS, NO_CONS},
          {NO_CONS, NO_CONS, NO_CONS, NO_CONS, SMALLER, GREATER, NO_CONS, NO_CONS},
          {NO_CONS, NO_CONS, NO_CONS, NO_CONS, GREATER, NO_CONS, NO_CONS, SMALLER},
          {SMALLER, NO_CONS, NO_CONS, NO_CONS, NO_CONS, NO_CONS, NO_CONS, NO_CONS}},
         .v_cons =  //
         {{NO_CONS, NO_CONS, NO_CONS, GREATER, GREATER, GREATER, GREATER, NO_CONS, NO_CONS},
          {NO_CONS, GREATER, GREATER, GREATER, GREATER, NO_CONS, NO_CONS, GREATER, GREATER},
          {NO_CONS, NO_CONS, NO_CONS, NO_CONS, NO_CONS, NO_CONS, NO_CONS, NO_CONS, GREATER},
          {SMALLER, NO_CONS, GREATER, NO_CONS, NO_CONS, SMALLER, NO_CONS, NO_CONS, NO_CONS},
          {SMALLER, SMALLER, NO_CONS, NO_CONS, NO_CONS, NO_CONS, NO_CONS, NO_CONS, GREATER},
          {NO_CONS, NO_CONS, NO_CONS, NO_CONS, NO_CONS, SMALLER, NO_CONS, NO_CONS, GREATER},
          {NO_CONS, GREATER, NO_CONS, NO_CONS, NO_CONS, NO_CONS, NO_CONS, NO_CONS, NO_CONS},
          {NO_CONS, GREATER, SMALLER, SMALLER, NO_CONS, NO_CONS, SMALLER, GREATER, SMALLER}}};

    printf("Initial Puzzle:\n");
    int initial_board[MAX_N][MAX_N];
    memcpy(initial_board, puzzle.board, sizeof(initial_board));
    print_board(&puzzle, initial_board);

    compute_pc_lists(&puzzle);

    printf("Possible colors for each cell:\n");
    for (int row = 0; row < puzzle.size; row++) {
        for (int col = 0; col < puzzle.size; col++) {
            printf("Cell [%d][%d]: ", row, col);
            for (int i = 0; i < puzzle.pc_lengths[row][col]; i++) {
                printf("%d ", puzzle.pc_list[row][col][i]);
            }
            printf("\n");
        }
    }

    int solution[MAX_N][MAX_N] = {0};
    if (color_g(&puzzle, solution, 0, 0)) {
        printf("\nSolution found:\n");
        print_board(&puzzle, solution);
    } else {
        printf("\nNo solution exists.\n");
    }

    return 0;
}