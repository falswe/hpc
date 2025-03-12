#!/bin/bash
# Compile the parallel Futoshiki solver with MPI and OpenMP

# Find the MPI compiler wrapper
MPICC=$(which mpicc)
if [ -z "$MPICC" ]; then
    echo "Error: MPI compiler not found. Make sure MPI is installed."
    exit 1
fi

# Set compiler flags
CFLAGS="-O3 -fopenmp"

# Compile each source file
$MPICC $CFLAGS -c comparison.c -o comparison.o
$MPICC $CFLAGS -c futoshiki.c -o futoshiki.o 
$MPICC $CFLAGS -c main.c -o main.o

# Link the object files
$MPICC $CFLAGS comparison.o futoshiki.o main.o -o futoshiki_parallel -lm

echo "Build complete: futoshiki_parallel"