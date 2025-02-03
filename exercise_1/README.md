# Parallel Programming on Game of Life

The `exercise_1/` folder contains all the files, subdirectories, and results related to the implementation.

The structure of the directory is the following: 

- **`data/`** – Scalability results in `.csv` format.
  - **`ordered/`** – Results from the **ordered evolution** version.
  - **`static/`** – Results from the **static evolution** version.

- **`include/`** – Header files included in `main.c`.

- **`jobs/`** – Shell scripts to compile and run the game on **SLURM**.
  - **`ordered/`** – Shell scripts for the **scalability studies** of ordered evolution.
  - **`static/`** – Shell scripts for the **scalability studies** of static evolution.

- **`plot/`** – Scalability result plots in `.png` format.

- **`src/`** – Source code for the implementation.
  - **`init.c`** – Initializes the game by creating the grid.
  - **`main.c`** – Main function that processes input arguments and calls functions to initialize and run the game.
  - **`ordered.c`** – Serial and parallel implementations of **ordered evolution**.
  - **`read_write_pgm_image.c`** – Functions to read and write `.pgm` image files.
  - **`static.c`** – Serial and parallel implementations of **static evolution**.

- **`Makefile`** – Compilation script for `main.c`.

- **`analysis.ipynb`** – Jupyter Notebook for **scalability analysis**.

## Reproducing the Results

To reproduce the results, simply run the provided shell scripts on **ORFEO**, as in the following example:

```sh
sbatch static_omp.sh
```

This script will:
1) Load the MPI module.
2) Compile the main source file.
3) Execute the respective scalability test.
4) Save the results in `.csv` files.

For plots just run all the cells in `analysis.ipynb`.

## IMPORTANT: Update `exec_dir`

Before running the scripts, modify the `exec_dir` variable inside the shell scripts to match your own path (the directory where the `Makefile` is located).