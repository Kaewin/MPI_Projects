#include <mpi.h>
#include <stdio.h>

void my_reduce_sum(int* send_data, int* recv_data, int root, MPI_Comm comm) {
    int rank, size;
    MPI_Comm_rank(comm, &rank);
    MPI_Comm_size(comm, &size);
    
    if (rank == root) {
        // TODO: Start with my own value, then receive from others and sum
        *recv_data = *send_data;

        for (int i = 0; i < size; i++) {
            if (i != root) {
                int temp;
                // Receive and add to result
                MPI_Recv(&temp, 1, MPI_INT, i, 0, comm, MPI_STATUS_IGNORE);
                *recv_data += temp;
            }
        }
    } else {
        // TODO: Send my value to root
        MPI_Send(send_data, 1, MPI_INT, root, 0, comm);
    }
}

int main(int argc, char** argv) {
    MPI_Init(&argc, &argv);
    
    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    
    // Each rank contributes their rank number + 1
    int my_value = rank + 1;
    int result = 0;
    
    printf("Rank %d contributing: %d\n", rank, my_value);
    
    // Your reduce
    my_reduce_sum(&my_value, &result, 0, MPI_COMM_WORLD);
    
    if (rank == 0) {
        printf("\nMy reduce result: %d\n", result);
        printf("Expected: %d\n", (size * (size + 1)) / 2);  // Sum of 1+2+...+N
    }
    
    // Compare with MPI_Reduce
    int result2 = 0;
    MPI_Reduce(&my_value, &result2, 1, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);
    
    if (rank == 0) {
        printf("MPI_Reduce result: %d\n", result2);
    }
    
    MPI_Finalize();
    return 0;
}