#!/bin/bash
#SBATCH --job-name=OMP_Scalability
#SBATCH --partition=THIN
#SBATCH --nodes=1                      # 1 node
#SBATCH --ntasks-per-node=1            # 1 MPI task per socket
#SBATCH --cpus-per-task=24             # Number of cores per socket
#SBATCH --time=02:00:00                # Adjust based on your needs
#SBATCH --output=OMP_scalability.out   # Output file

module load openMPI/4.1.6/gnu/14.2.1

# OpenMP settings
export OMP_PLACES=cores
export OMP_PROC_BIND=close

# Matrix sizes to test
matrix_sizes=(1000 5000 10000 25000)
n=100            # Number of evolution steps
s=0           # Snapshot frequency (set to 0 for no snapshots)
e=1      # Evolution type: 1 for static, 0 for ordered

# CSV output file
csv="static_omp_results.csv"
echo "matrix_size,threads,runtime,mean_time" > $csv

# Loop over matrix sizes
for size in "${matrix_sizes[@]}"; do
    # Initialize the matrix
    mpirun --map-by socket --bind-to socket -np 1 ./main.x -i -k $size -f "playground_${size}.pgm"

    # Test with varying number of threads
    for threads in $(seq 1 24); do
        export OMP_NUM_THREADS=$threads

        # Capture output from the program
        output=$(mpirun --map-by socket --bind-to socket -np 1 ./main.x -r -f "playground_${size}.pgm" -e $e -n $n -s $s)

        # Extract runtime and mean_time from the output
        runtime=$(echo "$output" | grep -o 'Runtime: [0-9.]*' | cut -d' ' -f2)
        mean_time=$(echo "$output" | grep -o 'Mean_Time: [0-9.]*' | cut -d' ' -f2)

        # Append to the CSV file
        echo "$size,$threads,$runtime,$mean_time" >> $csv
    done
done

module purge
