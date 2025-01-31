#!/bin/bash
#SBATCH --job-name=Weak_Scalability
#SBATCH --partition=THIN
#SBATCH --account=dssc
#SBATCH --nodes=2                    
#SBATCH --ntasks-per-node=12           
#SBATCH --time=02:00:00                
#SBATCH --output=weak_scalability.out

module load openMPI/4.1.6/gnu/14.2.1

exec_dir="/u/dssc/lmusini/FHPC/exercise_1"
output_dir="$exec_dir/data/ordered"

srun $exec_dir make clean
srun  make -C $exec_dir 

csv="$output_dir/ordered_weakmpi_results.csv"
echo "playground_size,mpi_task,runtime,mean_time" > $csv

# OpenMP settings
export OMP_PLACES=cores
export OMP_PROC_BIND=close
export OMP_NUM_THREADS=12

playground_sizes=(10000 14142 17320 20000 22360 24494 26458 28284)
#playground_sizes=(5000 7071 8660 10000 11180 12247 13228 14142)       # Playground sizes
n=50                                        # Number of evolution steps
s=0                                         # Snapshot frequency (set to 0 for snapshot only at the end)
e=0                                         # Evolution type: 0 for ordered, 1 for static


for task in $(seq 1 8) ; do
  
    size=${playground_sizes[$((task - 1))]}

    mpirun --map-by socket --bind-to socket -np 1 $exec_dir/main.x -i -k $size -f "playground_${size}.pgm"

    for run in $(seq 1 5); do
        output=$(mpirun --map-by socket --bind-to socket -np $task $exec_dir/main.x -r -f "playground_${size}.pgm" -e $e -n $n -s $s)

        runtime=$(echo "$output" | grep -o 'Runtime: [0-9.]*' | cut -d' ' -f2)
        mean_time=$(echo "$output" | grep -o 'Mean_Time: [0-9.]*' | cut -d' ' -f2)

        echo "$size,$task,$runtime,$mean_time" >> $csv
    done

    rm -f "playground_${size}.pgm"
done

module purge