#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include "bitmap.h"

/*
 * Main filter loop.
 * This function is responsible for doing the following:
 *   1. Read all pixels into a 2D array.
 *   2. Process bundary pixels and Apply edge detection kernel for non-boundary pixels
 *   3. Write out 3*3 pixels at a time.
 *
 */

void edge_detection_filter(Bitmap *bmp) {
    Pixel **rows = malloc(bmp->height * sizeof(Pixel *));
    if (!rows) {
        perror("Failed to allocate memory for rows");
        return;
    }

    for (int i = 0; i < bmp->height; i++) {
        rows[i] = malloc(bmp->width * sizeof(Pixel));
        if (!rows[i]) {
            perror("Failed to allocate memory for a row");
            for (int k = 0; k < i; k++) {
                free(rows[k]);
            }
            free(rows);
            return;
        }

        if (fread(rows[i], sizeof(Pixel), bmp->width, stdin) != bmp->width) {
            perror("Failed to read pixels");
            for (int k = 0; k <= i; k++) {
                free(rows[k]);
            }
            free(rows);
            return;
        }        
    }

    for (int i = 0; i < bmp->height; i++) {
        for (int j = 0; j < bmp->width; j++) {
            Pixel new_pixel;

            if (i == 0 || i == bmp->height - 1 || j == 0 || j == bmp->width - 1) {
                // Use the pixel from the inner adjacent position for boundary pixels
                int inner_i = (i == 0) ? 1 : (i == bmp->height - 1) ? bmp->height - 2 : i;
                int inner_j = (j == 0) ? 1 : (j == bmp->width - 1) ? bmp->width - 2 : j;
                new_pixel = rows[inner_i][inner_j];
            } else {
                // Apply edge detection kernel for non-boundary pixels
                Pixel window[3][3];
                for (int y = -1; y <= 1; y++) {
                    for (int x = -1; x <= 1; x++) {
                        window[y + 1][x + 1] = rows[i + y][j + x];
                    }
                }
                new_pixel = apply_edge_detection_kernel(window[0], window[1], window[2]);
            }

            if (fwrite(&new_pixel, sizeof(Pixel), 1, stdout) != 1) {
                perror("Failed to write pixel");
                for (int k = 0; k < bmp->height; k++) {
                    free(rows[k]);
                }
                free(rows);
                return;
            }
        }
    }
    
    for (int i = 0; i < bmp->height; i++) {
        free(rows[i]);
    }
    free(rows);
}

int main() {
    // Run the filter program with copy_filter to process the pixels.
    // You shouldn't need to change this implementation.
    run_filter(edge_detection_filter, 1);
    return 0;
}