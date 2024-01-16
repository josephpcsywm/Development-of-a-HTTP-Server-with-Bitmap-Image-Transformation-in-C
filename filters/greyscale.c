#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include "bitmap.h"

/*
 * Main filter loop.
 * This function is responsible for doing the following:
 *   1. Read in pixels one at a time.
 *   2. make the pixel greyscale by averaging the red, green, and blue values.
 *   3. Immediately write out each pixel.
 *
 */
void greyscale_filter(Bitmap *bmp) {
    Pixel pixel;
    size_t total = bmp->height * bmp->width;

    for (size_t i = 0; i < total; i++) {
        if (fread(&pixel, sizeof(Pixel), 1, stdin) != 1) {
            perror("Failed to read pixel");
            return;
        }

        unsigned char average = (pixel.red + pixel.green + pixel.blue) / 3;
        pixel.red = pixel.green = pixel.blue = average;

        if (fwrite(&pixel, sizeof(Pixel), 1, stdout) != 1) {
            perror("Failed to write pixel");
            return;
        }
    }
}
int main() {
    // Run the filter program with greyscale_filte to process the pixels.
    // You shouldn't need to change this implementation.
    run_filter(greyscale_filter, 1);
    return 0;
}