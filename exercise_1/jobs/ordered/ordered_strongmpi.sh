#!/bin/bash
#SBATCH --job-name=Strong_Scalability
#SBATCH --partition=THIN
#SBATCH --account=dssc
#SBATCH --nodes=4                    
#SBATCH --ntasks-per-node=2           
#SBATCH --time=02:00:00                
#SBATCH --output=strong_scalability.out

module load openMPI/4.1.6/gnu/14.2.1

exec_dir="/u/dssc/lmusini/FHPC/exercise_1"
output_dir="$exec_dir/data/ordered"

srun $exec_dir make clean
srun  make -C $exec_dir 

csv="$output_dir/ordered_strongmpi_results.csv"
echo "playground_size,mpi_task,runtime,mean_time" > $csv

# OpenMP settings
export OMP_PLACES=cores
export OMP_PROC_BIND=close
export OMP_NUM_THREADS=1

playground_sizes=(1000 5000 10000)     # Playground sizes 
n=50                                   # Number of evolution steps
s=0                                    # Snapshot frequency (set to 0 for snapshot only at the end)
e=0                                    # Evolution type: 0 for ordered, 1 for static

# Loop over matrix sizes
for size in "${playground_sizes[@]}"; do
  
    mpirun --map-by socket --bind-to core -np 1 ./main.x -i -k $size -f "playground_${size}.pgm"

    # Loop over number of MPI tasks
    for task in $(seq 1 8) ; do

        for run in $(seq 1 5); do
            output=$(mpirun --map-by node --bind-to core -np $task ./main.x -r -f "playground_${size}.pgm" -e $e -n $n -s $s)

            runtime=$(echo "$output" | grep -o 'Runtime: [0-9.]*' | cut -d' ' -f2)
            mean_time=$(echo "$output" | grep -o 'Mean_Time: [0-9.]*' | cut -d' ' -f2)

            echo "$size,$task,$runtime,$mean_time" >> $csv
        done
    done

    rm -f "playground_${size}.pgm"
done


module purge