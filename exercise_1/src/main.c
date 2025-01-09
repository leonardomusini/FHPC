#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mpi.h>
#include <omp.h>
#include <getopt.h>
#include <sys/time.h>

#include "read_write_pgm.h"
#include "init.h"
#include "ordered.h"
#include "static.h"

#define MAX_FILENAME_LENGTH 256

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

    struct timeval start_time, end_time;

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
            if (rank == 0) {
                fprintf(stderr, "Error: Unknown option -%c\n", c);
            }
            MPI_Finalize();
            return EXIT_FAILURE;
        }
    }

    if (!fname) {
        fname = (char *)malloc(MAX_FILENAME_LENGTH);
        snprintf(fname, MAX_FILENAME_LENGTH, "game_of_life.pgm");
    }

    MPI_Init(&argc, &argv);

    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    if (action == INIT) {
        if (k <= 0 || fname[0] == '\0') {
            if (rank == 0) {
                fprintf(stderr, "Error: Invalid parameters for initialization.\n");
            }
            MPI_Finalize();
            return EXIT_FAILURE;
        }

        unsigned char *playground = (unsigned char *)malloc(k * k * sizeof(unsigned char));
        initialize_playground(playground, k, k);

        if (rank == 0) {
            write_pgm_image(playground, 255, k, k, fname);
            printf("Playground initialized and saved to %s\n", fname);
        }

        free(playground);
    } else if (action == RUN) {
        if (fname[0] == '\0' || n <= 0 || (e != ORDERED && e != STATIC)) {
            if (rank == 0) {
                fprintf(stderr, "Error: Invalid parameters for running the playground.\n");
            }
            MPI_Finalize();
            return EXIT_FAILURE;
        }

        void *playground = NULL;
        int maxval;
        int width, height;
        read_pgm_image(&playground, &maxval, &width, &height, fname);
        if (!playground || width <= 0 || height <= 0) {
            if (rank == 0) {
                fprintf(stderr, "Error: Failed to read playground from %s\n", fname);
            }
            MPI_Finalize();
            return EXIT_FAILURE;
        }

        int block_height = height / size + (rank < height % size);
        unsigned char *local_playground = (unsigned char *)malloc(block_height * width * sizeof(unsigned char));
        MPI_Scatterv(playground, block_height * width, MPI_UNSIGNED_CHAR, local_playground, block_height * width, MPI_UNSIGNED_CHAR, 0, MPI_COMM_WORLD);

        gettimeofday(&start_time, NULL);

        if (e == ORDERED) {
            ordered_evolution(local_playground, width, height, block_height, n, s);
        } else {
            static_evolution(local_playground, width, height, block_height, n, s);
        }

        gettimeofday(&end_time, NULL);

        double runtime = (end_time.tv_sec - start_time.tv_sec) + (end_time.tv_usec - start_time.tv_usec) / 1e6;
        double mean_time = runtime / n;

        // Print runtime and mean_time to stdout
        if (rank == 0) {
            printf("Runtime: %f\n", runtime);
            printf("Mean_Time: %f\n", mean_time);
        }

        free(local_playground);
        if (rank == 0) free(playground);
    } else {
        if (rank == 0) {
            fprintf(stderr, "Error: No action specified. Use -i or -r.\n");
        }
        MPI_Finalize();
        return EXIT_FAILURE;
    }

    if (fname) {
        free(fname);
    }

    MPI_Finalize();
    return EXIT_SUCCESS;
}
