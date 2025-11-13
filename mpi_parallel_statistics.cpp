#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

int main(int argc, char** argv) {
    MPI_Init(&argc, &argv);
    
    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    
    // Each process generates 100 random numbers
    srand(time(NULL) + rank);
    int local_data[100];
    for (int i = 0; i < 100; i++) {
        local_data[i] = rand() % 1000;
    }
    
    // Calculate LOCAL statistics
    int local_sum = 0;
    int local_min = local_data[0];
    int local_max = local_data[0];
    
    for (int i = 0; i < 100; i++) {
        local_sum += local_data[i];
        if (local_data[i] < local_min) local_min = local_data[i];
        if (local_data[i] > local_max) local_max = local_data[i];
    }
    
    printf("Rank %d: local sum=%d, min=%d, max=%d\n", 
           rank, local_sum, local_min, local_max);
    
    // TODO: Compute GLOBAL statistics using MPI_Reduce
    int global_sum = 0;
    int global_min = 0;
    int global_max = 0;
    
    // Hint: Use MPI_Reduce with MPI_SUM, MPI_MIN, MPI_MAX
    MPI_Reduce(&local_sum, &global_sum, 1, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);
    MPI_Reduce(&local_min, &global_min, 1, MPI_INT, MPI_MIN, 0, MPI_COMM_WORLD);
    MPI_Reduce(&local_max, &global_max, 1, MPI_INT, MPI_MAX, 0, MPI_COMM_WORLD);
    
    if (rank == 0) {
        double global_avg = global_sum / (double)(size * 100);
        printf("\n=== GLOBAL STATISTICS ===\n");
        printf("Processes: %d\n", size);
        printf("Total numbers: %d\n", size * 100);
        printf("Sum: %d\n", global_sum);
        printf("Average: %.2f\n", global_avg);
        printf("Min: %d\n", global_min);
        printf("Max: %d\n", global_max);
    }
    
    MPI_Finalize();
    return 0;
}