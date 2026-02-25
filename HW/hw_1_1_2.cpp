#include <mpi.h>
#include <stdio.h>

#define NUM_PROCS 4

int main(int argc, char *argv[]) {
    int rank, size, value = 0;
    int right, left;
    int trips;

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    if (size != NUM_PROCS) {
        if (rank == 0) printf("Please run with %d processes.\n", NUM_PROCS);
        MPI_Finalize();
        return 1;
    }

    right = (rank + 1) % size;
    left  = (rank - 1 + size) % size;

    if (rank == 0) {
        printf("How many times around the ring? ");
        scanf("%d", &trips);
    }

    MPI_Bcast(&trips, 1, MPI_INT, 0, MPI_COMM_WORLD);

    for (int t = 0; t < trips; t++) {
        if (rank == 0) {
            MPI_Send(&value, 1, MPI_INT, right, 0, MPI_COMM_WORLD);
            MPI_Recv(&value, 1, MPI_INT, left,  0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        } else {
            MPI_Recv(&value, 1, MPI_INT, left,  0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            value++;
            MPI_Send(&value, 1, MPI_INT, right, 0, MPI_COMM_WORLD);
        }
    }

    if (rank == 0) {
        printf("Final value after %d trips: %d\n", trips, value);
    }

    MPI_Finalize();
    return 0;
}