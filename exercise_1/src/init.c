#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <omp.h>

#define ALIVE 255
#define DEAD 0

void initialize_playground(unsigned char *playground, int width, int height) {
    // Seed the random number generator
    unsigned int seed = (unsigned int)time(NULL);

    #pragma omp parallel
    {
        unsigned int thread_seed = seed + omp_get_thread_num();
        #pragma omp for collapse(2) schedule(static)
        for (int y = 0; y < height; y++) {
            for (int x = 0; x < width; x++) {
                playground[y * width + x] = (rand_r(&thread_seed) % 2 == 0) ? ALIVE : DEAD;
            }
        }
    }
}