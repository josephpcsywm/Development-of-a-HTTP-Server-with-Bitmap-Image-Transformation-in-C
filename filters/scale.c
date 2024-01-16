#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include "bitmap.h"
#include <math.h>


/*
 * Main filter loop.
 * This function is responsible for doing the following:
 *   1. read the entire image.
 *   2. scales it up by the scale factor which store in bmp->scaleFactor
 *   3. write out the scaled image.
 */
void scale_filter(Bitmap *bmp) {
    int original_height = bmp->height / bmp->scaleFactor;
    int original_width = bmp->width / bmp->scaleFactor;

    Pixel **original = malloc(original_height * sizeof(Pixel *));
    if (!original) {
        perror("Failed to allocate memory for original image");
        return;
    }

    for (int i = 0; i < original_height; i++) {
        original[i] = malloc(original_width * sizeof(Pixel));
        if (!original[i]) {
            perror("Failed to allocate memory for a row");
            for (int k = 0; k < i; k++) {
                free(original[k]);
            }
            free(original);
            return;
        }

        if (fread(original[i], sizeof(Pixel), original_width, stdin) != original_width) {
            perror("Failed to read pixels");
            for (int k = 0; k <= i; k++) {
                free(original[k]);
            }
            free(original);
            return;
        }
    }

    for (int i = 0; i < bmp->height; i++) {
        for (int j = 0; j < bmp->width; j++) {
            int orig_row = i / bmp->scaleFactor;
            int orig_col = j / bmp->scaleFactor;
            Pixel scaled_pixel = original[orig_row][orig_col];

            if (fwrite(&scaled_pixel, sizeof(Pixel), 1, stdout) != 1) {
                perror("Failed to write scaled pixel");
                for (int k = 0; k < original_height; k++) {
                    free(original[k]);
                }
                free(original);
                return;
            }
        }
    }
    
    for (int i = 0; i < original_height; i++) {
        free(original[i]);
    }
    free(original);

}

int main() {
    // Run the filter program with copy_filter to process the pixels.
    // You shouldn't need to change this implementation.
    run_filter(scale_filter, 2);
    return 0;
}