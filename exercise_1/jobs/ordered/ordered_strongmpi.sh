#!/bin/bash
#SBATCH --job-name=Strong_Scalability
#SBATCH --partition=THIN
#SBATCH --account=dssc
#SBATCH --nodes=3                    
#SBATCH --ntasks-per-node=24           
#SBATCH --time=02:00:00   
#SBATCH --exclusive             
#SBATCH --output=strong_scalability.out

module load openMPI/4.1.6/gnu/14.2.1

exec_dir="/u/dssc/lmusini/FHPC/exercise_1"
output_dir="$exec_dir/data/ordered"
csv="$output_dir/ordered_strongmpi_results.csv"
echo "playground_size,mpi_task,runtime,mean_time" > $csv

# OpenMP settings
export OMP_PLACES=cores
export OMP_PROC_BIND=close
export OMP_NUM_THREADS=1

playground_sizes=(10000 15000 20000)                    # Playground sizes 
mpi_tasks=(1 2 4 8 12 16 20 24 28 32 36 40 44 48 52 56 60 64 68 72)    # Number of MPI tasks
n=50                                                    # Number of evolution steps
s=0                                                     # Snapshot frequency (set to 0 for snapshot only at the end)
e=0                                                     # Evolution type: 0 for ordered, 1 for static

# Loop over matrix sizes
for size in "${playground_sizes[@]}"; do
  
    mpirun --map-by socket --bind-to core -np 1 ./main.x -i -k $size -f "playground_${size}.pgm"

    # Loop over number of MPI tasks
    for task in "${mpi_tasks[@]}" ; do

        for run in $(seq 1 5); do
            output=$(mpirun --map-by node --bind-to core -np $task ./main.x -r -f "playground_${size}.pgm" -e $e -n $n -s $s)

            runtime=$(echo "$output" | grep -o 'Runtime: [0-9.]*' | cut -d' ' -f2)
            mean_time=$(echo "$output" | grep -o 'Mean_Time: [0-9.]*' | cut -d' ' -f2)

            echo "$size,$task,$runtime,$mean_time" >> $csv
        done
    done
done

rm -f "playground_*.pgm"

module purge