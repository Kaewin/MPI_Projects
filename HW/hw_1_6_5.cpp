#include <iostream>
#include <mpi.h>

int main (int argc, char** argv) {
    MPI_Init(&argc, &argv);

    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    int recvbuff[size];
    int sendbuff[size];

    if (size < 4) {
        std::cout << "At least four processes are needed for this program." << std::endl;
        MPI_Finalize();
        return 1;
    }

    for(int i = 0; i < size; i++) {
        sendbuff[i] = rank * 300 + i * 100;
    }
    
    std::cout << "Starting values on process: " << rank << " " << sendbuff[0] << " " << sendbuff[1] << " " << sendbuff[2] << " " << std::endl;

    MPI_Alltoall(&sendbuff, 1, MPI_INT, recvbuff, 1, MPI_INT, MPI_COMM_WORLD);

    std::cout << "Values collected on process: " << rank << " " << recvbuff[0] << " " << recvbuff[1] << " " << recvbuff[2] << " " << std::endl;

    MPI_Finalize();
    return 0;
}