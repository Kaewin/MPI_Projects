#include <iostream>
#include <mpi.h>

int main (int argc, char** argv) {
    MPI_Init(&argc, &argv);

    int rank, size, value, result;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    // Each process sets its value to its rank 
    value = rank;
    int recvbuff[size];
    result = 0;

    if (size < 4) {
        std::cout << "At least four processes are needed for this program." << std::endl;
        MPI_Finalize();
        return 1;
    }

    MPI_Gather(&value, 1, MPI_INT, recvbuff, 1, MPI_INT, 0, MPI_COMM_WORLD);

    MPI_Bcast(recvbuff, size, MPI_INT, 0, MPI_COMM_WORLD);
    
    std::cout << "Values collected on process: " << rank << " " << recvbuff[0] << recvbuff[1] << recvbuff[2] << std::endl;

    MPI_Finalize();
    return 0;
}