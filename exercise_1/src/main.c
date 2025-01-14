#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mpi.h>
#include <omp.h>
#include <getopt.h>
#include <sys/time.h>
#include <time.h>

#include "read_write_pgm_image.h"
#include "init.h"
#include "ordered.h"
#include "static.h"

#define MAX_FILENAME_LENGTH 256
#define ALIVE 255
#define DEAD 0

#define INIT 1
#define RUN 2
#define K_DEFAULT 100
#define ORDERED 0
#define STATIC 1

int main(int argc, char *argv[]) {

    int action = 0;
    int k = K_DEFAULT;
    int e = ORDERED;
    int n = 20;
    int s = 0;
    char *fname = NULL;

    char *optstring = "irk:e:f:n:s:";
    int c;

    struct timespec start_time, end_time;

    while ((c = getopt(argc, argv, optstring)) != -1) {
        switch (c) {
        case 'i':
            action = INIT;
            break;
        case 'r':
            action = RUN;
            break;
        case 'k':
            k = atoi(optarg);
            break;
        case 'e':
            e = atoi(optarg);
            break;
        case 'f':
            fname = (char *)malloc(strlen(optarg) + 1);
            strcpy(fname, optarg);
            break;
        case 'n':
            n = atoi(optarg);
            break;
        case 's':
            s = atoi(optarg);
            break;
        default:
            printf("argument -%c not known\n", c ); break;
        }
    }

    int rank, size;
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    if (action == INIT) {

        unsigned char *playground = NULL;
        if (rank == 0) {
            playground = (unsigned char *)malloc(k * k * sizeof(unsigned char));
            initialize_playground(playground, k, k);
            write_pgm_image(playground, 255, k, k, fname);
            printf("Playground initialized and saved to %s\n", fname);
            free(playground);
        }
    } else if (action == RUN) {
       
        if (size > 1) {

            clock_gettime(CLOCK_MONOTONIC, &start_time);

            unsigned char *playground = NULL;
            int maxval, width, height;

            if (rank == 0) {
                read_pgm_image((void **)&playground, &maxval, &width, &height, fname);
            }

            // Broadcasting
            MPI_Bcast(&n, 1, MPI_INT, 0, MPI_COMM_WORLD);
            MPI_Bcast(&s, 1, MPI_INT, 0, MPI_COMM_WORLD);
            MPI_Bcast(&e, 1, MPI_INT, 0, MPI_COMM_WORLD);
            MPI_Bcast(&width, 1, MPI_INT, 0, MPI_COMM_WORLD);
            MPI_Bcast(&height, 1, MPI_INT, 0, MPI_COMM_WORLD);

            // Calculate block sizes and displacements
            int base_height = height / size;
            int remainder = height % size;
            int block_height = height / size + (rank < remainder);
            unsigned char *local_playground = (unsigned char *)malloc(block_height * width * sizeof(unsigned char));
            int *proc_counts = NULL, *displs = NULL;

            if (rank == 0) {
                proc_counts = (int *)malloc(size * sizeof(int));
                displs = (int *)malloc(size * sizeof(int));

                for (int i = 0; i < size; i++) {
                    proc_counts[i] = (i < remainder ? base_height + 1 : base_height) * width;
                    displs[i] = (i == 0 ? 0 : displs[i - 1] + proc_counts[i - 1]);
                }
            }

            MPI_Scatterv(playground, proc_counts, displs, MPI_UNSIGNED_CHAR, local_playground, block_height * width, MPI_UNSIGNED_CHAR, 0, MPI_COMM_WORLD);

            if (rank == 0) {
                free(playground);
            }

            if (e == ORDERED) {
                mpi_ordered_evolution(local_playground, width, height, block_height, n, s, rank, size, proc_counts, displs);
            } else if (e == STATIC) {
                mpi_static_evolution(local_playground, width, height, block_height, n, s, rank, size, proc_counts, displs);
            }

            clock_gettime(CLOCK_MONOTONIC, &end_time);

            double runtime = (end_time.tv_sec - start_time.tv_sec) + (end_time.tv_nsec - start_time.tv_nsec) / 1e9;
            double mean_time = runtime / n;

            if (rank == 0) {
                printf("Runtime: %f\n", runtime);
                printf("Mean_Time: %f\n", mean_time);
                free(proc_counts);
                free(displs);
            }

            free(local_playground);

        } else {
          
            clock_gettime(CLOCK_MONOTONIC, &start_time);

            unsigned char *playground = NULL;
            int maxval, width, height;

            read_pgm_image((void **)&playground, &maxval, &width, &height, fname);

            if (e == ORDERED) {
                ordered_evolution(playground, width, height, n, s);
            } else if (e == STATIC) {
                static_evolution(playground, width, height, n, s);
            }

            clock_gettime(CLOCK_MONOTONIC, &end_time);

            double runtime = (end_time.tv_sec - start_time.tv_sec) + (end_time.tv_nsec - start_time.tv_nsec) / 1e9;
            double mean_time = runtime / n;
            printf("Runtime: %f\n", runtime);
            printf("Mean_Time: %f\n", mean_time);

            free(playground);
        }
    }

    if (fname) {
        free(fname);
    }

    MPI_Finalize();
    return EXIT_SUCCESS;
}