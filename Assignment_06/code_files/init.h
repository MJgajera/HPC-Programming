#ifndef INIT_H
#define INIT_H
#include <stdio.h>

// CHANGED: Using Structure of Arrays (SoA) instead of Array of Structures (AoS)
// This dramatically improves CPU cache utilization.
typedef struct {
    double *x;
    double *y;
} Points;

extern int GRID_X, GRID_Y, NX, NY;
extern int NUM_Points, Maxiter;
extern double dx, dy;

// New functions to handle SoA allocation
void allocate_points(Points *points, int n);
void free_points(Points *points);
void read_points(FILE *file, Points *points);

#endif