#!/bin/bash
#SBATCH --job-name=Static_Strong_Scalability
#SBATCH --partition=THIN
#SBATCH --nodes=4                    
#SBATCH --ntasks-per-node=2          
#SBATCH --cpus-per-task=12             # Number of cores per socket
#SBATCH --time=02:00:00               
#SBATCH --output=static_strong_scalability.out

module load openMPI/4.1.6/gnu/14.2.1

# OpenMP settings
export OMP_PLACES=cores
export OMP_PROC_BIND=close
export OMP_NUM_THREADS=24              # Number of OpenMP threads per MPI task

# Matrix sizes 
matrix_sizes=(1000 5000 10000 25000)
# Number of evolution steps
n=100            
# Snapshot frequency (set to 0 for snapshot only at the end)
s=0    
# Evolution type: 0 for ordered, 1 for static        
e=1                      

# Output CSV file
csv="static_strong_results.csv"
echo "matrix_size,mpi_tasks,runtime,mean_time" > $csv

for size in "${matrix_sizes[@]}"; do
    # Initialize the matrix
    mpirun --map-by socket --bind-to socket -np 1 ./main.x -i -k $size -f "playground_${size}.pgm"

    for mpi_tasks in 1 2 4 8; do
        # Run the program and capture output
        output=$(mpirun --map-by socket --bind-to socket -np $mpi_tasks ./main.x -r -f "playground_${size}.pgm" -e $evolution_type -n $nsteps -s $snap_freq)

        # Extract runtime and mean_time
        runtime=$(echo "$output" | grep -o 'Runtime: [0-9.]*' | cut -d' ' -f2)
        mean_time=$(echo "$output" | grep -o 'Mean_Time: [0-9.]*' | cut -d' ' -f2)

        # Append results to CSV
        echo "$size,$mpi_tasks,$runtime,$mean_time" >> $csv
    done
done

module purge
