#include <stdio.h>
#include <stdlib.h>
#include "init.h"

void allocate_points(Points *points, int n) {
    // Allocating flat, contiguous arrays for x and y coordinates separately
    points->x = (double*)malloc(n * sizeof(double));
    points->y = (double*)malloc(n * sizeof(double));
}

void free_points(Points *points) {
    free(points->x);
    free(points->y);
}

void read_points(FILE *file, Points *points) {
    // Read the binary file directly into the separate SoA arrays
    for (int i = 0; i < NUM_Points; i++) {
        fread(&points->x[i], sizeof(double), 1, file);
        fread(&points->y[i], sizeof(double), 1, file);
    }
}