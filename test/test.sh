#!/bin/bash

# set
# o number of chunks (~nodes) 
# o cores per chunk
# o shared memory per chunk (?)
#PBS -l select=1:ncpus=4:mem=2gb

# set max execution time
#PBS -l walltime=0:05:00

# set the queue
#PBS -q short_cpuQ

module load mpich-3.2
mpirun.actual -n 4 ./test.out
