#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include "bitmap.h"

/*
 * Main filter loop.
 * This function is responsible for doing the following:
 *   1. Read in pixels one at a time (because copy is a pixel-by-pixel transformation).
 *   2. Immediately write out each pixel.
 *
 * Note that this function should allocate space only for a single Pixel;
 * do *not* store more than one Pixel at a time, it isn't necessary here!
 */
void copy_filter(Bitmap *bmp) {
    Pixel pixel;
    size_t total = bmp->height * bmp->width;

    for (size_t i = 0; i < total; i++) {
        if (fread(&pixel, sizeof(Pixel), 1, stdin) != 1) {
            perror("Failed to read pixel");
            return;
        }
        if (fwrite(&pixel, sizeof(Pixel), 1, stdout) != 1) {
            perror("Failed to write pixel");
            return;
        }
        
    }
}


int main() {
    // Run the filter program with copy_filter to process the pixels.
    // You shouldn't need to change this implementation.
    run_filter(copy_filter, 1);
    return 0;
}