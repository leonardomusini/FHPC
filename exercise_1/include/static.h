#ifndef STATIC_H
#define STATIC_H

void mpi_static_evolution(unsigned char *playground, int width, int height, int block_height, int tot_steps, int snap_step,
                          int rank, int size, int *proc_counts, int *displs);
void static_evolution(unsigned char *playground, int width, int height, int tot_steps, int snap_step);

#endif // STATIC_H