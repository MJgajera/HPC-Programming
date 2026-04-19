#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <omp.h>
#include "utils.h"

void interpolation(double *mesh_value, Points *points) {
    int grid_size = GRID_X * GRID_Y;
    memset(mesh_value, 0, grid_size * sizeof(double));

    double inv_dx = (double)NX;
    double inv_dy = (double)NY;
    double cell_area = dx * dy;

    // CHANGED: Direct SoA access and Fine-Grained Global Atomics
    #pragma omp parallel for schedule(static)
    for (int p = 0; p < NUM_Points; p++) {
        // Access coordinates from the SoA directly
        double px = points->x[p];
        double py = points->y[p];

        int i = (int)(px * inv_dx);
        int j = (int)(py * inv_dy);

        i = (i >= NX) ? NX - 1 : ((i < 0) ? 0 : i);
        j = (j >= NY) ? NY - 1 : ((j < 0) ? 0 : j);

        // Calculate distances with optimized math
        double d_x = (px * inv_dx) - i;
        double d_y = (py * inv_dy) - j;
        double inv_d_x = 1.0 - d_x;
        double inv_d_y = 1.0 - d_y;

        double w00 = inv_d_x * inv_d_y * cell_area;
        double w10 = d_x * inv_d_y * cell_area;
        double w01 = inv_d_x * d_y * cell_area;
        double w11 = d_x * d_y * cell_area;

        int base_idx = j * GRID_X + i;

        // No more local mesh or critical sections. 
        // We use fast hardware-level atomic additions on the global grid.
        #pragma omp atomic
        mesh_value[base_idx] += w00;
        
        #pragma omp atomic
        mesh_value[base_idx + 1] += w10;
        
        #pragma omp atomic
        mesh_value[base_idx + GRID_X] += w01;
        
        #pragma omp atomic
        mesh_value[base_idx + GRID_X + 1] += w11;
    }
}

void save_mesh(double *mesh_value) {
    FILE *fd = fopen("Mesh_parallel.out", "w");
    if (!fd) {
        printf("Error creating Mesh_parallel.out\n");
        exit(1);
    }
    for (int i = 0; i < GRID_Y; i++) {
        for (int j = 0; j < GRID_X; j++) {
            fprintf(fd, "%lf ", mesh_value[i * GRID_X + j]);
        }
        fprintf(fd, "\n");
    }
    fclose(fd);
}