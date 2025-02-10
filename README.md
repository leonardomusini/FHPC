# FHPC - Exam Assignments
 
This repository contains the solutions of the exam assignments of the course "Foundations of High Performance Computing" offered by Master's Degree in Data Science and Scientific Computing at the University of Trieste.

Each folder contains its respective assignment and `FHPC_report.pdf`includes the reports for both exercises.

Some scripts in this repository are slightly modified versions of code from the official course [GitHub repository](https://github.com/Foundations-of-HPC/Foundations_of_HPC_2022/tree/main/Assignment).

## Exercise 1: Parallel Programming on Game of Life

This exercise involves the implementation of the famous [Conway's Game of Life](https://en.wikipedia.org/wiki/Conway%27s_Game_of_Life)
 using a **hybrid parallel computing** approach with **MPI (Message Passing Interface)**, to split the grid and assign a block to each MPI process,  and **OpenMP (Open Multi-Processing)**, to utilize multi-core advantages on each MPI task.

The game is implemented in two types of evolutional strategies:

- **Ordered Evolution**: cells are upgraded sequentially in a row-major order.
- **Static Evolution**: cells in the grid are evaluated simultaneously, and their updates are applied collectively at the end of each timestep.
 
 To assess the performance of the algorithm, three types of scalability studies are conducted:
 
 - **OpenMP Scalability**: the number of MPI tasks is fixed, while the number of threads per task is increased up to the number of cores present in the socket.
 - **Strong MPI Scalability**: the size of the matrix is kept fixed, while the number of MPI tasks is increased on multiple nodes.
 - **Weak MPI Scalability**: the size of the matrix increase proportionally with the number of MPI tasks.

## Exercise 2: Comparing MKL, OpenBLAS and BLIS on matrix-matrix multiplication

In this exercise, we compare the performance of three BLAS libraries (**Intel MKL**, **OpenBLAS** and **BLIS**) on matrix multiplication, computed using **level 3 BLAS function** called `gemm`. In the study the computations are performed using **single** and **double precision**, exploring different thread allocation policies on two [ORFEO](https://orfeo-doc.areasciencepark.it/) nodes: **THIN** and **EPYC**. 

To analyze the performance, two scalability tests are conducted:

- **Size Scalability**: increasing the size of the matrices while keeping the number of cores fixed.
- **Core Scalability**: increasing the number of cores while keeping the matrix size constant. 