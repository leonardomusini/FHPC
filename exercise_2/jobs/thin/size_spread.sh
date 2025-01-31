#!/bin/bash 
#SBATCH --partition=THIN 
#SBATCH --job-name=Size_Spread_scalability
#SBATCH --nodes=1 
#SBATCH --ntasks-per-node=1 
#SBATCH --cpus-per-task=24 
#SBATCH --mem=200gb
#SBATCH --time=02:00:00 
#SBATCH --exclusive
#SBATCH --output=size_spread.out

module load openBLAS/0.3.26-omp
export LD_LIBRARY_PATH=/u/dssc/lmusini/intel/oneapi/mkl/2025.0/lib:$LD_LIBRARY_PATH
export LD_LIBRARY_PATH=/u/dssc/lmusini/myblis/lib:$LD_LIBRARY_PATH

export OMP_PLACES=cores
export OMP_PROC_BIND=spread
export OMP_NUM_THREADS=12
export BLIS_NUM_THREADS=12 

exec_dir="/u/dssc/lmusini/FHPC/exercise_2"
output_dir="$exec_dir/data/thin/size/spread"

precisions=('float' 'double')
implementations=('oblas' 'mkl' 'blis')

srun $exec_dir make clean
srun -n1 make -C $exec_dir cpu 

for prec in "${precisions[@]}"; do

    for impl in "${implementations[@]}"; do

        csv="$output_dir/${prec}_${impl}_size_spread_results.csv"
        echo "size,runtime,GFLOPS" > $csv

        for size in $(seq 2000 1000 20000); do

            for run in $(seq 1 5); do

                output=$(srun -n1 --cpus-per-task=12 $exec_dir/gemm_${prec}_${impl}.x $size $size $size)     
            
                runtime=$(echo "$output" | grep -oP 'Elapsed time \K[0-9.]+' | head -n1)
                gflops=$(echo "$output" | grep -oP '[0-9.]+ GFLOPS' | grep -oP '[0-9.]+')
 
                echo "$size,$runtime,$gflops" >> $csv

            done
        done
    done
done

module purge
