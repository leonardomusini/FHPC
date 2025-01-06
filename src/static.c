#include "static_evolution.h"
#include <stdio.h>
#include <stdlib.h>
#include <omp.h>
#include <mpi.h>
#include "read_write_pgm.h"

#define ALIVE 255
#define DEAD 0

void static_evolution(unsigned char *playground, int width, int block_height, int tot_steps, int snap_step) {
    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    // Allocate halo row buffers
    unsigned char *row_buffer = (unsigned char *)malloc(2 * width * sizeof(unsigned char));
    unsigned char *halo_row_top = row_buffer;
    unsigned char *halo_row_bottom = row_buffer + width;

    // Determine neighbors
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

    MPI_Request communication_requests[4];

    // Loop through all steps
    for (int step = 0; step < tot_steps; step++) {

        // Non-blocking halo exchange
        MPI_Irecv(halo_row_top, width, MPI_UNSIGNED_CHAR, neighbor_top, 0, MPI_COMM_WORLD, &communication_requests[0]);
        MPI_Irecv(halo_row_bottom, width, MPI_UNSIGNED_CHAR, neighbor_bottom, 1, MPI_COMM_WORLD, &communication_requests[1]);
        MPI_Isend(playground, width, MPI_UNSIGNED_CHAR, neighbor_top, 1, MPI_COMM_WORLD, &communication_requests[2]);
        MPI_Isend(playground + (block_height - 1) * width, width, MPI_UNSIGNED_CHAR, neighbor_bottom, 0, MPI_COMM_WORLD, &communication_requests[3]);

        MPI_Waitall(4, communication_requests, MPI_STATUSES_IGNORE);

        // Parallel static evolution using OpenMP
        #pragma omp parallel for collapse(2) schedule(static)
        for (int row = 0; row < block_height; row++) {
            for (int col = 0; col < width; col++) {
                int alive_neighbors = 0;

                // Calculate alive neighbors
                for (int i = 0; i < 8; i++) {
                    int offset_x = neighbor_offsets[i][0];
                    int offset_y = neighbor_offsets[i][1];

                    int neighbor_x = (col + offset_x + width) % width;
                    int neighbor_y = row + offset_y;

                    unsigned char neighbor_value;
                    if (neighbor_y < 0) {
                        neighbor_value = halo_row_top[neighbor_x];
                    } else if (neighbor_y >= block_height) {
                        neighbor_value = halo_row_bottom[neighbor_x];
                    } else {
                        neighbor_value = playground[neighbor_y * width + neighbor_x];
                    }

                    alive_neighbors += (neighbor_value == ALIVE);
                }

                int cell_index = row * width + col;
                // Apply static evolution rules
                playground[cell_index] = ((playground[cell_index] == ALIVE && (alive_neighbors == 2 || alive_neighbors == 3)) ||
                                            (playground[cell_index] == DEAD && alive_neighbors == 3))
                                               ? ALIVE
                                               : DEAD;
            }
        }

        // Save snapshot if needed
        if (snap_step > 0 && step % snap_step == 0) {
            save_snapshot(playground, width, block_height, step, rank);
        }

        if (rank == 0 && step % 100 == 0) {
            printf("Step %d completed\n", step);
        }
    }

    // Final snapshot
    if (snap_step == 0 || tot_steps % snap_step != 0) {
        save_snapshot(playground, width, block_height, tot_steps, rank);
    }

    free(row_buffer);
}