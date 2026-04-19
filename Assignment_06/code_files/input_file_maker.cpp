#include <stdio.h>
#include <stdlib.h>
#include <time.h>

void generate_input_file(const char* filename, int NX, int NY, int NUM_Points, int Maxiter) {
    FILE *fp = fopen(filename, "wb");
    if (fp == NULL) {
        perror("Error creating file");
        exit(EXIT_FAILURE);
    }

    fwrite(&NX, sizeof(int), 1, fp);
    fwrite(&NY, sizeof(int), 1, fp);
    fwrite(&NUM_Points, sizeof(int), 1, fp);
    fwrite(&Maxiter, sizeof(int), 1, fp);

    srand((unsigned int)time(NULL));

    for (int iter = 0; iter < Maxiter; iter++) {
        for (int i = 0; i < NUM_Points; i++) {
            double x = (double)rand() / RAND_MAX;
            double y = (double)rand() / RAND_MAX;
            fwrite(&x, sizeof(double), 1, fp);
            fwrite(&y, sizeof(double), 1, fp);
        }
    }   

    fclose(fp);
    printf("Generated '%s' successfully.\n", filename);
}

int main(int argc, char **argv) {
    // Allows fast creation via command line: 
    // ./maker <filename> <NX> <NY> <Points> <Iter>
    if (argc == 6) {
        generate_input_file(argv[1], atoi(argv[2]), atoi(argv[3]), atoi(argv[4]), atoi(argv[5]));
    } else {
        // Fallback to interactive prompts if no arguments are provided
        int NX, NY, NUM_Points, Maxiter;
        char filename[256];

        printf("Enter filename (e.g., input_A.bin): ");
        scanf("%255s", filename);

        printf("Enter grid dimensions (NX NY): ");
        scanf("%d %d", &NX, &NY);

        printf("Enter number of particles: ");
        scanf("%d", &NUM_Points);

        printf("Enter number of iterations: ");
        scanf("%d", &Maxiter);

        generate_input_file(filename, NX, NY, NUM_Points, Maxiter);
    }

    return 0;
}
