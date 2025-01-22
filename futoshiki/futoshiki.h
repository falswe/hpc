#ifndef FUTOSHIKI_H
#define FUTOSHIKI_H

#include "comparison.h"  // For SolverStats

SolverStats solve_puzzle(const char* filename, bool use_precoloring, bool print_details);

#endif  // FUTOSHIKI_H
