#include <mpi.h>
#include <stdio.h>

void my_broadcast(int* data, int root, MPI_Comm comm) {
    int rank, size;
    MPI_Comm_rank(comm, &rank);
    MPI_Comm_size(comm, &size);
    
    // If I'm root, send to everyone
    // If I'm not root, receive from root
    if (rank == root) {
        for (int i = 0; i < size; i++) {
            if (i != root) {
                MPI_Send(data, 1, MPI_INT, i, 0, comm);
            }
        }
    } else {
        MPI_Recv(data, 1, MPI_INT, root, 0, comm, MPI_STATUS_IGNORE);
    }
}

int main(int argc, char** argv) {
    MPI_Init(&argc, &argv);
    
    int rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    
    int data;
    if (rank == 0) {
        data = 42;  // Root has the data
        printf("Rank 0 broadcasting: %d\n", data);
    }
    
    // Your broadcast
    my_broadcast(&data, 0, MPI_COMM_WORLD);
    
    printf("Rank %d received: %d\n", rank, data);
    
    // Compare with real MPI_Bcast
    int data2 = (rank == 0) ? 99 : 0;
    double start = MPI_Wtime();
    MPI_Bcast(&data2, 1, MPI_INT, 0, MPI_COMM_WORLD);
    double elapsed = MPI_Wtime() - start;
    
    if (rank == 0) {
        printf("MPI_Bcast took: %f seconds\n", elapsed);
    }
    
    MPI_Finalize();
    return 0;
}