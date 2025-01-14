#ifndef READ_WRITE_PGM_IMAGE_H
#define READ_WRITE_PGM_IMAGE_H

#define XWIDTH 256
#define YWIDTH 256
#define MAXVAL 255

void write_pgm_image(void *image, int maxval, int xsize, int ysize, const char *image_name);
void read_pgm_image(void **image, int *maxval, int *xsize, int *ysize, const char *image_name);

#endif // READ_WRITE_PGM_IMAGE_H