#!/bin/bash

# This script runs the Futoshiki solver with different configurations
# to evaluate performance and scalability

# Define the puzzle file to use
PUZZLE_FILE="9x9_extreme3_initial.txt"

# Define process and thread configurations to test
MPI_PROCS_LIST="1 2 4 8 16"
OMP_THREADS_LIST="1 2 4 8"

# Output file for results
OUTPUT_FILE="performance_results.csv"

# Create header for results file
echo "MPI_Processes,OMP_Threads,Execution_Time,Speedup" > $OUTPUT_FILE

# Get baseline time with 1 process and 1 thread
export OMP_NUM_THREADS=1
/usr/bin/time -f "%e" mpirun -np 1 ./futoshiki_parallel $PUZZLE_FILE > /dev/null 2> baseline_time.txt
BASELINE_TIME=$(cat baseline_time.txt)
echo "Baseline execution time (1 process, 1 thread): $BASELINE_TIME seconds"

# Run tests with different configurations
for procs in $MPI_PROCS_LIST; do
    for threads in $OMP_THREADS_LIST; do
        echo "Testing with $procs MPI processes and $threads OpenMP threads..."
        
        # Set OpenMP threads
        export OMP_NUM_THREADS=$threads
        
        # Run the solver and measure time
        /usr/bin/time -f "%e" mpirun -np $procs ./futoshiki_parallel $PUZZLE_FILE > /dev/null 2> time_output.txt
        EXEC_TIME=$(cat time_output.txt)
        
        # Calculate speedup
        SPEEDUP=$(echo "scale=2; $BASELINE_TIME / $EXEC_TIME" | bc)
        
        # Append results to the output file
        echo "$procs,$threads,$EXEC_TIME,$SPEEDUP" >> $OUTPUT_FILE
        
        echo "  Execution time: $EXEC_TIME seconds (Speedup: ${SPEEDUP}x)"
    done
done

echo "Performance testing completed. Results saved to $OUTPUT_FILE"