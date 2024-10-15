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
module load mpich-3.2
# build
mpicc ~/hpc/reduce/all-reduce.c -g -Wall -std=c99 -o ~/hpc/reduce/all-reduce
# run
mpirun.actual -n 8 ~/hpc/reduce/all-reduce
