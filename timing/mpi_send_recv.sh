#!/bin/bash

# Notes
# -----
# o absolute paths for consistency across nodes

# max walltime 6h
#PBS -q short_cpuQ
# expected timespan for execution
#PBS -l walltime=00:01:00
# chunks (~nodes) : cores per chunk : shared memory per chunk (?)
#PBS -l select=2:ncpus=1:mem=2gb

# get dependencies
module load mpich-3.2
# build
mpicc ~/hpc/timing/mpi_send_recv.c -g -Wall -std=c99 -o ~/hpc/timing/mpi_send_recv
# run
mpirun.actual -n 2 ~/hpc/timing/mpi_send_recv

