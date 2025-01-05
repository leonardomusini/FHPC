#include "ordered_evolution.h"
#include <stdio.h>
#include <stdlib.h>
#include <omp.h>
#include <mpi.h>

#define ALIVE 255
#define DEAD 0

void ordered_evolution(unsigned char *current_grid, int grid_width, int subgrid_height, int total_steps) {
    int process_rank, total_processes;
    MPI_Comm_rank(MPI_COMM_WORLD, &process_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &total_processes);

    unsigned char *row_buffer = (unsigned char *)malloc(2 * grid_width * sizeof(unsigned char));
    unsigned char *ghost_row_top = row_buffer;
    unsigned char *ghost_row_bottom = row_buffer + grid_width;

    int neighbor_top = (process_rank == 0) ? total_processes - 1 : process_rank - 1;
    int neighbor_bottom = (process_rank == total_processes - 1) ? 0 : process_rank + 1;

    for (int step = 0; step < total_steps; step++) {
        MPI_Request communication_requests[4];

        MPI_Irecv(ghost_row_top, grid_width, MPI_UNSIGNED_CHAR, neighbor_top, 0, MPI_COMM_WORLD, &communication_requests[0]);
        MPI_Irecv(ghost_row_bottom, grid_width, MPI_UNSIGNED_CHAR, neighbor_bottom, 1, MPI_COMM_WORLD, &communication_requests[1]);
        MPI_Isend(current_grid, grid_width, MPI_UNSIGNED_CHAR, neighbor_top, 1, MPI_COMM_WORLD, &communication_requests[2]);
        MPI_Isend(current_grid + (subgrid_height - 1) * grid_width, grid_width, MPI_UNSIGNED_CHAR, neighbor_bottom, 0, MPI_COMM_WORLD, &communication_requests[3]);

        MPI_Waitall(4, communication_requests, MPI_STATUSES_IGNORE);

        for (int mpi_order = 0; mpi_order < total_processes; mpi_order++) {
            if (mpi_order == process_rank) {
                for (int row = 0; row < subgrid_height; row++) {
                    #pragma omp parallel for ordered schedule(static)
                    for (int col = 0; col < grid_width; col++) {
                        #pragma omp ordered
                        {
                            int alive_neighbors = 0;
                            for (int offset_x = -1; offset_x <= 1; offset_x++) {
                                for (int offset_y = -1; offset_y <= 1; offset_y++) {
                                    if (offset_x == 0 && offset_y == 0) continue;

                                    int neighbor_x = (col + offset_x + grid_width) % grid_width;
                                    int neighbor_y = row + offset_y;

                                    unsigned char neighbor_value;
                                    if (neighbor_y < 0) {
                                        neighbor_value = ghost_row_top[neighbor_x];
                                    } else if (neighbor_y >= subgrid_height) {
                                        neighbor_value = ghost_row_bottom[neighbor_x];
                                    } else {
                                        neighbor_value = current_grid[neighbor_y * grid_width + neighbor_x];
                                    }

                                    alive_neighbors += (neighbor_value == ALIVE);
                                }
                            }

                            int cell_index = row * grid_width + col;
                            current_grid[cell_index] = ((current_grid[cell_index] == ALIVE && (alive_neighbors == 2 || alive_neighbors == 3)) ||
                                                        (current_grid[cell_index] == DEAD && alive_neighbors == 3))
                                                           ? ALIVE
                                                           : DEAD;
                        }
                    }
                }
            }
            MPI_Barrier(MPI_COMM_WORLD); // Ensure ranks execute sequentially
        }

        if (process_rank == 0 && step % 100 == 0) {
            printf("Step %d completed\n", step);
        }
    }

    free(row_buffer);
}
