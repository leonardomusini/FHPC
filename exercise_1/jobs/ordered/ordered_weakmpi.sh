#!/bin/bash
#SBATCH --job-name=Weak_Scalability
#SBATCH --partition=THIN
#SBATCH --nodes=4                    # Use up to 4 nodes
#SBATCH --ntasks-per-node=2          # 1 MPI task per socket
#SBATCH --cpus-per-task=12           # Number of cores per socket
#SBATCH --time=02:00:00              # Adjust as needed
#SBATCH --output=weak_scalability.out

module load openMPI/4.1.6/gnu/14.2.1

# OpenMP settings
export OMP_PLACES=cores
export OMP_PROC_BIND=close

# Initial matrix size
initial_size=5000
# Number of evolution steps
n=100            
# Snapshot frequency (set to 0 for snapshot only at the end)
s=0    
# Evolution type: 0 for ordered, 1 for static        
e=0

# Output CSV file
csv="ordered_weak_results.csv"
echo "mpi_tasks,new_size,runtime,mean_time" > $csv

# Loop over number of MPI tasks
for mpi_tasks in 1 2 4 8 16; do
    # Calculate matrix size (square) to keep workload per task constant
    new_size=$((initial_size * sqrt(mpi_tasks)))

    # Initialize the matrix
    mpirun --map-by socket --bind-to socket -np 1 ./main.x -i -k $new_size -f "playground_${new_size}.pgm"

    # Run the program and capture output
    output=$(mpirun --map-by socket --bind-to socket -np $mpi_tasks ./main.x -r -f "playground_${new_size}.pgm" -e $evolution_type -n $nsteps -s $snap_freq)

    # Extract runtime and mean_time
    runtime=$(echo "$output" | grep -o 'Runtime: [0-9.]*' | cut -d' ' -f2)
    mean_time=$(echo "$output" | grep -o 'Mean_Time: [0-9.]*' | cut -d' ' -f2)

    # Append results to CSV
    echo "$mpi_tasks,$new_size,$runtime,$mean_time" >> $csv
done

module purge
