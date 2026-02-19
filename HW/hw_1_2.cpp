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

    printf("Process %d initialized\n", rank);
    fflush(stdout);

    if (size != NUM_PROCS) {
        if (rank == 0) printf("Please run with %d processes.\n", NUM_PROCS);
        MPI_Finalize();
        return 1;
    }

    right = (rank + 1) % size;
    left  = (rank - 1 + size) % size;

    printf("Process %d: left neighbor = %d, right neighbor = %d\n", rank, left, right);
    fflush(stdout);

    if (rank == 0) {
        printf("How many times around the ring? ");
        fflush(stdout);
        scanf("%d", &trips);
        printf("Process 0: will do %d trips\n", trips);
        fflush(stdout);
    }

    MPI_Bcast(&trips, 1, MPI_INT, 0, MPI_COMM_WORLD);
    printf("Process %d: received trips = %d via Bcast\n", rank, trips);
    fflush(stdout);

    for (int t = 0; t < trips; t++) {
        if (rank == 0) {
            printf("Process 0: starting trip %d, sending value %d to process %d\n", t, value, right);
            fflush(stdout);
            MPI_Send(&value, 1, MPI_INT, right, 0, MPI_COMM_WORLD);
            printf("Process 0: waiting to receive from process %d\n", left);
            fflush(stdout);
            MPI_Recv(&value, 1, MPI_INT, left, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            printf("Process 0: completed trip %d, received value %d\n", t, value);
            fflush(stdout);
        } else {
            printf("Process %d: trip %d, waiting to receive from process %d\n", rank, t, left);
            fflush(stdout);
            MPI_Recv(&value, 1, MPI_INT, left, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            value++;
            printf("Process %d: trip %d, received and incremented to %d, sending to process %d\n", rank, t, value, right);
            fflush(stdout);
            MPI_Send(&value, 1, MPI_INT, right, 0, MPI_COMM_WORLD);
        }
    }

    if (rank == 0) {
        printf("Final value after %d trips: %d\n", trips, value);
    }

    MPI_Finalize();
    return 0;
}
