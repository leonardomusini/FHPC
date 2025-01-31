#include <stdio.h>
#include <stdlib.h>
#include <omp.h>
#include <mpi.h>

#include "read_write_pgm_image.h"

#define ALIVE 255
#define DEAD 0

void mpi_static_evolution(unsigned char *playground, int width, int height, int block_height, int tot_steps, int snap_step,
                          int rank, int size, int *proc_counts, int *displs) {
    
    // Needed for snapshot saving
    unsigned char *global_playground = NULL;
    if (rank == 0) {
        global_playground = (unsigned char *)malloc(width * height * sizeof(unsigned char));
    }
    
    // Allocate buffers
    unsigned char *row_buffer = (unsigned char *)malloc(2 * width * sizeof(unsigned char));
    unsigned char *halo_top = row_buffer;
    unsigned char *halo_bottom = row_buffer + width;
    unsigned char *next_playground = (unsigned char *)malloc(block_height * width * sizeof(unsigned char));

    // Determine neighbors of MPI processes
    int neighbor_top = (rank == 0) ? size - 1 : rank - 1;
    int neighbor_bottom = (rank == size - 1) ? 0 : rank + 1;

    // Precompute neighbor offsets for performance
    int neighbor_offsets[8][2] = {
        {-1, -1}, // Top-left
        {-1,  0}, // Top
        {-1,  1}, // Top-right
        { 0, -1}, // Left
        { 0,  1}, // Right
        { 1, -1}, // Bottom-left
        { 1,  0}, // Bottom
        { 1,  1}  // Bottom-right
    };

    MPI_Request requests[4];

    for (int step = 0; step < tot_steps; step++) {
  
        MPI_Irecv(halo_top, width, MPI_UNSIGNED_CHAR, neighbor_top, 0, MPI_COMM_WORLD, &requests[0]);
        MPI_Irecv(halo_bottom, width, MPI_UNSIGNED_CHAR, neighbor_bottom, 1, MPI_COMM_WORLD, &requests[1]);
        MPI_Isend(playground, width, MPI_UNSIGNED_CHAR, neighbor_top, 1, MPI_COMM_WORLD, &requests[2]);
        MPI_Isend(playground + (block_height - 1) * width, width, MPI_UNSIGNED_CHAR, neighbor_bottom, 0, MPI_COMM_WORLD, &requests[3]);

        // Compute inner rows (rows not dependent on halo rows)
        #pragma omp parallel for collapse(2) schedule(static)
        for (int row = 1; row < block_height - 1; row++) {
            for (int col = 0; col < width; col++) {
                int alive_neighbors = 0;

                // Calculate alive neighbors for inner rows
                for (int i = 0; i < 8; i++) {
                    int offset_x = neighbor_offsets[i][0];
                    int offset_y = neighbor_offsets[i][1];

                    int neighbor_x = (col + offset_x + width) % width;
                    int neighbor_y = row + offset_y;

                    alive_neighbors += (playground[neighbor_y * width + neighbor_x] == ALIVE);
                }

                int cell_index = row * width + col;
                next_playground[cell_index] = ((playground[cell_index] == ALIVE && (alive_neighbors == 2 || alive_neighbors == 3)) ||
                                               (playground[cell_index] == DEAD && alive_neighbors == 3)) ? ALIVE : DEAD;
            }
        }

        MPI_Waitall(4, requests, MPI_STATUSES_IGNORE);

        // Compute halo rows (top and bottom rows)
        #pragma omp parallel for collapse(2) schedule(static)
        for (int row = 0; row < block_height; row += (block_height - 1)) { // Top and bottom halo rows
            for (int col = 0; col < width; col++) {
                int alive_neighbors = 0;

                // Calculate alive neighbors for halo rows
                for (int i = 0; i < 8; i++) {
                    int offset_x = neighbor_offsets[i][0];
                    int offset_y = neighbor_offsets[i][1];

                    int neighbor_x = (col + offset_x + width) % width;
                    int neighbor_y = row + offset_y;

                    unsigned char neighbor_value;
                    if (neighbor_y < 0) {
                        neighbor_value = halo_top[neighbor_x];
                    } else if (neighbor_y >= block_height) {
                        neighbor_value = halo_bottom[neighbor_x];
                    } else {
                        neighbor_value = playground[neighbor_y * width + neighbor_x];
                    }

                    alive_neighbors += (neighbor_value == ALIVE);
                }

                int cell_index = row * width + col;
                next_playground[cell_index] = ((playground[cell_index] == ALIVE && (alive_neighbors == 2 || alive_neighbors == 3)) ||
                                               (playground[cell_index] == DEAD && alive_neighbors == 3)) ? ALIVE : DEAD;
            }
        }

        // Swap playgrounds
        unsigned char *temp = playground;
        playground = next_playground;
        next_playground = temp;

        // Save snapshots 
        if (snap_step > 0 && step % snap_step == 0) {
           
            MPI_Gatherv(playground, block_height * width, MPI_UNSIGNED_CHAR, global_playground, proc_counts, displs, MPI_UNSIGNED_CHAR, 0, MPI_COMM_WORLD);
          
            if (rank == 0) {
                char snapshot_filename[256];
                snprintf(snapshot_filename, sizeof(snapshot_filename), "snapshot_%05d.pgm", step);
                write_pgm_image(global_playground, 255, width, height, snapshot_filename);
                printf("Global snapshot saved: %s\n", snapshot_filename);
            }
        }

        // if (rank == 0 && step % 100 == 0) {
           // printf("Step %d completed\n", step);
        // }
    }

    // Save final snapshot
    if (snap_step == 0 || (snap_step > 0 && tot_steps % snap_step != 0)) {
        MPI_Gatherv(playground, block_height * width, MPI_UNSIGNED_CHAR, global_playground, proc_counts, displs, MPI_UNSIGNED_CHAR, 0, MPI_COMM_WORLD);

        if (rank == 0) {
            char snapshot_filename[256];
            snprintf(snapshot_filename, sizeof(snapshot_filename), "snapshot_%05d.pgm", tot_steps);
            write_pgm_image(global_playground, 255, width, height, snapshot_filename);
            printf("Final snapshot saved: %s\n", snapshot_filename);
        }
    }

    // Free allocated memory
    free(row_buffer);
    free(next_playground);
    if (rank == 0) {
        free(global_playground);
    }
}

// --------------------------------------------------------------------------------------------

void static_evolution(unsigned char *playground, int width, int height, int tot_steps, int snap_step) {     // Non-MPI version
    // Neighbor offsets
    int neighbor_offsets[8][2] = {
        {-1, -1}, // Top-left
        {-1,  0}, // Top
        {-1,  1}, // Top-right
        { 0, -1}, // Left
        { 0,  1}, // Right
        { 1, -1}, // Bottom-left
        { 1,  0}, // Bottom
        { 1,  1}  // Bottom-right
    };

    // Buffer for the next state
    unsigned char *next_playground = (unsigned char *)malloc(width * height * sizeof(unsigned char));

    for (int step = 0; step < tot_steps; step++) {
        #pragma omp parallel for collapse(2) schedule(static)
        for (int row = 0; row < height; row++) {
            for (int col = 0; col < width; col++) {
                int alive_neighbors = 0;

                // Calculate alive neighbors
                for (int i = 0; i < 8; i++) {
                    int offset_x = neighbor_offsets[i][0];
                    int offset_y = neighbor_offsets[i][1];

                    int neighbor_x = (col + offset_x + width) % width;
                    int neighbor_y = (row + offset_y + height) % height;

                    alive_neighbors += (playground[neighbor_y * width + neighbor_x] == ALIVE);
                }

                // Apply rules of the game
                int cell_index = row * width + col;
                next_playground[cell_index] = ((playground[cell_index] == ALIVE && (alive_neighbors == 2 || alive_neighbors == 3)) ||
                                               (playground[cell_index] == DEAD && alive_neighbors == 3)) ? ALIVE : DEAD;
            }
        }

        // Swap playgrounds
        unsigned char *temp = playground;
        playground = next_playground;
        next_playground = temp;

        // Save snapshot 
        if (snap_step > 0 && step % snap_step == 0) {
            char snapshot_filename[256];
            snprintf(snapshot_filename, sizeof(snapshot_filename), "snapshot_%05d.pgm", step);
            write_pgm_image(playground, 255, width, height, snapshot_filename);
            printf("Snapshot saved: %s\n", snapshot_filename);
        }
    }

    // Save final snapshot
    if (snap_step == 0 || (snap_step > 0 && tot_steps % snap_step != 0)) {
        char snapshot_filename[256];
        snprintf(snapshot_filename, sizeof(snapshot_filename), "snapshot_%05d.pgm", tot_steps);
        write_pgm_image(playground, 255, width, height, snapshot_filename);
        printf("Snapshot saved: %s\n", snapshot_filename);
    }

    // Free allocated memory
    free(next_playground);
}