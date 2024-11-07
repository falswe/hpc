#!/bin/bash

# Notes
# -----
# o absolute paths for consistency across nodes

# max walltime 6h
#PBS -q short_cpuQ
# expected timespan for execution
#PBS -l walltime=00:01:00
# chunks (~nodes) : cores per chunk : shared memory per chunk (?)
#PBS -l select=4:ncpus=8:mem=2gb

# get dependencies
# module load mpich-3.2
module load openmpi-4.0.4
# build
# mpicc ~/hpc/hello_world_omp/hello_world_omp.c -g -Wall -std=c99 -o ~/hpc/hello_world_omp/hello_world_omp
gcc ~/hpc/hello_world_omp/hello_world_omp.c -g -Wall -fopenmp -std=c99 -o ~/hpc/hello_world_omp/hello_world_omp
# run
# mpirun.actual -n 4 ~/hpc/hello_world_omp/hello_world_omp
~/hpc/hello_world_omp/hello_world_omp 4