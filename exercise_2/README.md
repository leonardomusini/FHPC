# Comparing MKL, OpenBLAS and BLIS on matrix-matrix multiplication

The `exercise_2/` folder contains all the files, subdirectories, and results related to the scalability tests.

The structure of the directory is the following: 

- **`data/`** – Scalability results in `.csv` format.
  - **`epyc/`** – Results from the **EPYC** partition (the structure of the directory reflect the following one).
  - **`thin/`** – Results from the **THIN** partition.
     - **`core/`** – Results of the **core scalability** study (the structure of the directory reflect the following one).
     - **`size/`** – Results of the **size scalability** study.
         - **`close/`** – Results of the **close** thread affinity policy.
         - **`spread/`** – Results of the **spread** thread affinity policy.

- **`jobs/`** – Shell scripts to compile and run the matrix multiplication on **SLURM**.
  - **`epyc/`** – Shell scripts for each thread affinity policy and scalability test on **EPYC** partition.
  - **`thin/`** – Shell scripts for each thread affinity policy and scalability test on **THIN** partition.

- **`Makefile`** – Compilation script for `gemm.c`: it will create several executables, one for each BLAS library and computation precision.

- **`analysis.ipynb`** – Jupyter Notebook for **scalability analysis**.

- **`gemm.c`** – Main file that call `gemm` function with different precisions and measures performances.

## Reproducing the Results

To reproduce the results, simply run the provided shell scripts on **ORFEO**, as in the following example:

```sh
sbatch core_close.sh
```

This script will:
1) Load the three libraries.
2) Compile the source file, generating the executables.
3) Execute the respective scalability test.
4) Save the results in `.csv` files.

For plots just run all the cells in `analysis.ipynb`.

## IMPORTANT

Before running the scripts, modify the `exec_dir` variable inside the shell scripts to match your own path (the directory where the `Makefile` is located).

In addition, modify the `LD_LIBRARY_PATH` with the path of your own installed library.