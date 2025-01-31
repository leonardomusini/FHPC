#ifndef ORDERED_H
#define ORDERED_H

void mpi_ordered_evolution(unsigned char *playground, int width, int height, int block_height, int tot_steps, int snap_step,
                           int rank, int size, int *proc_counts, int *displs);
void ordered_evolution(unsigned char *playground, int width, int height, int tot_steps, int snap_step);

#endif // ORDERED_H