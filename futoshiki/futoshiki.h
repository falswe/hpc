#ifndef FUTOSHIKI_H
#define FUTOSHIKI_H

#include <stdbool.h>

#include "comparison.h"  // For SolverStats

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

// Original serial functions
SolverStats solve_puzzle(const char* filename, bool use_precoloring, bool print_details);
bool safe(const Futoshiki* puzzle, int row, int col, int solution[MAX_N][MAX_N], int color);
bool has_valid_neighbor(const Futoshiki* puzzle, int row, int col, int color, bool need_greater);
bool satisfies_inequalities(const Futoshiki* puzzle, int row, int col, int color);
void filter_possible_colors(Futoshiki* puzzle, int row, int col);
void process_uniqueness(Futoshiki* puzzle, int row, int col);
int compute_pc_lists(Futoshiki* puzzle, bool use_precoloring);
bool color_g(Futoshiki* puzzle, int solution[MAX_N][MAX_N], int row, int col);
void print_board(const Futoshiki* puzzle, int solution[MAX_N][MAX_N]);
bool parse_futoshiki(const char* input, Futoshiki* puzzle);
bool read_puzzle_from_file(const char* filename, Futoshiki* puzzle);
double get_time();

// New parallel functions
SolverStats solve_puzzle_parallel(const char* filename, bool use_precoloring, bool print_details,
                                  int rank, int size);
void broadcast_puzzle(Futoshiki* puzzle, int root);
void gather_solution(int solution[MAX_N][MAX_N], int puzzle_size, bool* found_solution,
                     int* found_by);

#endif  // FUTOSHIKI_H