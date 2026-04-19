#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <omp.h>
#include "utils.h"

double min_val, max_val;

// Interpolation (Scatter: Point -> Mesh)
void interpolation(double *mesh_value, Points *points) {
    int grid_size = GRID_X * GRID_Y;
    
    // CRITICAL: Clear the grid at the start of each iteration 
    // so we don't infinitely accumulate density from previous timesteps.
    memset(mesh_value, 0, grid_size * sizeof(double));

    double inv_dx = (double)NX;
    double inv_dy = (double)NY;
    double cell_area = dx * dy;

    // Use fast atomic operations to prevent race conditions
    #pragma omp parallel for schedule(static)
    for (int p = 0; p < NUM_Points; p++) {
        if (points[p].is_void) continue; // Skip inactive particles

        double x = points[p].x;
        double y = points[p].y;

        int i = (int)(x * inv_dx);
        int j = (int)(y * inv_dy);

        i = (i >= NX) ? NX - 1 : ((i < 0) ? 0 : i);
        j = (j >= NY) ? NY - 1 : ((j < 0) ? 0 : j);

        double d_x = (x * inv_dx) - i;
        double d_y = (y * inv_dy) - j;

        double inv_d_x = 1.0 - d_x;
        double inv_d_y = 1.0 - d_y;

        double w00 = inv_d_x * inv_d_y * cell_area;
        double w10 = d_x * inv_d_y * cell_area;
        double w01 = inv_d_x * d_y * cell_area;
        double w11 = d_x * d_y * cell_area;

        int base_idx = j * GRID_X + i;

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

void normalization(double *mesh_value) {
    int grid_size = GRID_X * GRID_Y;
    min_val = mesh_value[0];
    max_val = mesh_value[0];
    
    // Bulletproof manual reduction to find both Min and Max
    #pragma omp parallel
    {
        double local_min = mesh_value[0];
        double local_max = mesh_value[0];
        
        #pragma omp for schedule(static)
        for (int i = 0; i < grid_size; i++) {
            if (mesh_value[i] < local_min) local_min = mesh_value[i];
            if (mesh_value[i] > local_max) local_max = mesh_value[i];
        }
        
        #pragma omp critical
        {
            if (local_min < min_val) min_val = local_min;
            if (local_max > max_val) max_val = local_max;
        }
    }

    double range = max_val - min_val;
    if (range == 0.0) range = 1.0; // Prevent division by zero

    // Normalize grid strictly to the range [-1.0, 1.0] using Min-Max scaling
    #pragma omp parallel for schedule(static)
    for (int i = 0; i < grid_size; i++) {
        mesh_value[i] = 2.0 * (mesh_value[i] - min_val) / range - 1.0;
    }
}

// Mover (Gather: Mesh -> Point)
void mover(double *mesh_value, Points *points) {
    double inv_dx = (double)NX;
    double inv_dy = (double)NY;
    double cell_area = dx * dy;

    #pragma omp parallel for schedule(static)
    for (int p = 0; p < NUM_Points; p++) {
        if (points[p].is_void) continue;

        double x = points[p].x;
        double y = points[p].y;

        int i = (int)(x * inv_dx);
        int j = (int)(y * inv_dy);

        i = (i >= NX) ? NX - 1 : ((i < 0) ? 0 : i);
        j = (j >= NY) ? NY - 1 : ((j < 0) ? 0 : j);

        double d_x = (x * inv_dx) - i;
        double d_y = (y * inv_dy) - j;

        double inv_d_x = 1.0 - d_x;
        double inv_d_y = 1.0 - d_y;

        double w00 = inv_d_x * inv_d_y * cell_area;
        double w10 = d_x * inv_d_y * cell_area;
        double w01 = inv_d_x * d_y * cell_area;
        double w11 = d_x * d_y * cell_area;

        int base_idx = j * GRID_X + i;

        // Calculate the field force using the exact same weights
        double F_i = w00 * mesh_value[base_idx] +
                     w10 * mesh_value[base_idx + 1] +
                     w01 * mesh_value[base_idx + GRID_X] +
                     w11 * mesh_value[base_idx + GRID_X + 1];

        // Update particle position
        points[p].x += F_i * dx;
        points[p].y += F_i * dy;

        // Boundary check: Mark particles outside the domain as inactive
        if (points[p].x < 0.0 || points[p].x > 1.0 || 
            points[p].y < 0.0 || points[p].y > 1.0) {
            points[p].is_void = true;
        }
    }
}

void denormalization(double *mesh_value) {
    int grid_size = GRID_X * GRID_Y;
    double range = max_val - min_val;
    if (range == 0.0) range = 1.0;

    // Reverse the Min-Max scaling to restore original values
    #pragma omp parallel for schedule(static)
    for (int i = 0; i < grid_size; i++) {
        mesh_value[i] = (mesh_value[i] + 1.0) * range / 2.0 + min_val;
    }
}

long long int void_count(Points *points) {
    long long int voids = 0;
    #pragma omp parallel for reduction(+:voids)
    for (int i = 0; i < NUM_Points; i++) {
        voids += (int)points[i].is_void;
    }
    return voids;
}

void save_mesh(double *mesh_value) {
    FILE *fd = fopen("Mesh.out", "w");
    if (!fd) {
        printf("Error creating Mesh.out\n");
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
