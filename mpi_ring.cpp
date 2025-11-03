#include <iostream>
#include <mpi.h>
#include <cstring> // for std::strlen
#include <cstdio>  // for std::snprintf

int main(int argc, char** argv) {
    MPI_Init(&argc, &argv);

    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    if (size != 3) {
        if (rank == 0) std::cerr << "This program requires 3 MPI processes exactly.\n";
        MPI_Finalize();
        return 1;
    }

    int next_rank = (rank + 1) % size; // Next rank in the ring
    int prev_rank = (rank - 1 + size) % size; // Previous rank in the ring

    const int TAG = 0;
    const int MAX_MSG = 256;
    char buffer[MAX_MSG];
    const int iterations = 10;

    for (int i = 0; i < iterations; ++i) {
        if (rank == 0) {
            std::snprintf(buffer, MAX_MSG, "Message %d from rank 0", i);
            MPI_Send(buffer, (int)std::strlen(buffer) + 1, MPI_CHAR, next_rank, TAG, MPI_COMM_WORLD);
            MPI_Recv(buffer, MAX_MSG, MPI_CHAR, prev_rank, TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            std::cout << "Rank 0 received: " << buffer << std::endl;
        } else {
            MPI_Recv(buffer, MAX_MSG, MPI_CHAR, prev_rank, TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            std::cout << "Rank " << rank << " received: " << buffer << std::endl;
            std::snprintf(buffer, MAX_MSG, "Message %d from rank %d", i, rank);
            MPI_Send(buffer, (int)std::strlen(buffer) + 1, MPI_CHAR, next_rank, TAG, MPI_COMM_WORLD);
        }
    }

    MPI_Finalize();
    return 0;
}