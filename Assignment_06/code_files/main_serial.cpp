#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "init.h"
#include "utils.h"

int GRID_X, GRID_Y, NX, NY;
int NUM_Points, Maxiter;
double dx, dy;

int main(int argc, char **argv) {
    if (argc != 2) {
        printf("Usage: %s <input_file>\n", argv[0]);
        return 1;
    }

    FILE *file = fopen(argv[1], "rb");
    if (!file) {
        printf("Error opening input file\n");
        exit(1);
    }

    fread(&NX, sizeof(int), 1, file);
    fread(&NY, sizeof(int), 1, file);
    fread(&NUM_Points, sizeof(int), 1, file);
    fread(&Maxiter, sizeof(int), 1, file);

    GRID_X = NX + 1;
    GRID_Y = NY + 1;
    dx = 1.0 / NX;
    dy = 1.0 / NY;

    double *mesh_value = (double *) calloc(GRID_X * GRID_Y, sizeof(double));
    
    Points points;
    allocate_points(&points, NUM_Points);

    double total_time = 0.0;

    for (int iter = 0; iter < Maxiter; iter++) {
        read_points(file, &points);

        clock_t start = clock();
        interpolation(mesh_value, &points);
        clock_t end = clock();

        total_time += (double)(end - start) / CLOCKS_PER_SEC;
    }

    save_mesh(mesh_value);
    printf("%lf\n", total_time); 

    free(mesh_value);
    free_points(&points);
    fclose(file);

    return 0;
}