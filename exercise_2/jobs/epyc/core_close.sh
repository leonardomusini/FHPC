#!/bin/bash 
#SBATCH --partition=EPYC 
#SBATCH --job-name=Core_Close_scalability
#SBATCH --nodes=1 
#SBATCH --ntasks-per-node=1 
#SBATCH --cpus-per-task=128 
#SBATCH --mem=200gb
#SBATCH --time=02:00:00 
#SBATCH --exclusive
#SBATCH --output=core_close.out

module load openBLAS/0.3.26-omp
export LD_LIBRARY_PATH=/u/dssc/lmusini/intel/oneapi/mkl/2025.0/lib:$LD_LIBRARY_PATH
export LD_LIBRARY_PATH=/u/dssc/lmusini/myblis_epyc/lib:$LD_LIBRARY_PATH

export OMP_PLACES=cores
export OMP_PROC_BIND=close 

exec_dir="/u/dssc/lmusini/FHPC/exercise_2"
output_dir="$exec_dir/data/epyc/core/close"

precisions=('float' 'double')
implementations=('oblas' 'mkl' 'blis')
size=10000

srun $exec_dir make clean
srun -n1 make -C $exec_dir cpu 

for prec in "${precisions[@]}"; do

    for impl in "${implementations[@]}"; do

        csv="$output_dir/${prec}_${impl}_core_close_results.csv"
        echo "threads,runtime,GFLOPS" > $csv

        for threads in 1 2 $(seq 4 4 128); do

            export OMP_NUM_THREADS=$threads
            export BLIS_NUM_THREADS=$threads

            for run in $(seq 1 5); do

                output=$(srun -n1 --cpus-per-task=$threads $exec_dir/gemm_${prec}_${impl}.x $size $size $size)     
            
                runtime=$(echo "$output" | grep -oP 'Elapsed time \K[0-9.]+' | head -n1)
                gflops=$(echo "$output" | grep -oP '[0-9.]+ GFLOPS' | grep -oP '[0-9.]+')

                echo "$threads,$runtime,$gflops" >> $csv

            done
        done
    done
done

module purge