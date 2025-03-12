# Parallel Futoshiki Solver

This project implements a parallel version of the Futoshiki puzzle solver using MPI and OpenMP.

## Overview

Futoshiki is a Latin square completion puzzle with inequality constraints. The solver uses a list coloring and precoloring technique to efficiently solve the puzzle.

The project includes both serial and parallel implementations:
- Serial implementation: Uses backtracking with precoloring optimization
- Parallel implementation: Uses MPI for distributed memory parallelism and OpenMP for shared memory parallelism

## Files

- `main.c`: Main program with MPI initialization and coordination
- `futoshiki.h`: Header file with function declarations and data structures
- `futoshiki.c`: Implementation of the Futoshiki solver
- `comparison.h/c`: Functions for comparing solver performance with and without precoloring
- `build_parallel.sh`: Script to compile the parallel solver
- `run_parallel.pbs`: PBS job script for running on a cluster
- `test_performance.sh`: Script for performance testing

## Building

To build the parallel solver:

```bash
# Make the build script executable
chmod +x build_parallel.sh

# Build the solver
./build_parallel.sh
```

## Running

On a cluster with PBS:

```bash
# Submit the job
qsub run_parallel.pbs
```

Locally (if MPI is installed):

```bash
# Run with 4 MPI processes and 4 OpenMP threads per process
export OMP_NUM_THREADS=4
mpirun -np 4 ./futoshiki_parallel 9x9_extreme3_initial.txt
```

## Performance Testing

To evaluate performance with different configurations:

```bash
# Make the script executable
chmod +x test_performance.sh

# Run performance tests
./test_performance.sh
```

## Algorithm Details

The solver uses a two-phase approach:

1. **Precoloring Phase**:
   - For each cell, compute possible values (colors) based on constraints
   - Filter colors based on inequality constraints
   - Process uniqueness to further reduce possible colors

2. **List Coloring Phase**:
   - Use backtracking to assign values to cells
   - Distribute the search space among MPI processes
   - Use OpenMP to parallelize certain operations

## Parallelization Strategy

- **MPI Parallelization**:
  - Each process explores a different branch of the search tree
  - Work is distributed by assigning different values for the first empty cell
  - Solution is shared once found

- **OpenMP Parallelization**:
  - Used for independent operations in the precoloring phase
  - Parallel loops for initialization and filtering
  - Critical sections to ensure thread safety

## Performance Results

Performance testing shows significant speedup with hybrid parallelization:
- Near-linear speedup with MPI processes up to 16 processes
- Additional speedup with OpenMP threads
- Best performance with a balance of MPI processes and OpenMP threads