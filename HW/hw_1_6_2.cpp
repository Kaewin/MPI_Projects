#include <iostream>
#include <mpi.h>

using namespace std;

int main (int argc, char** argv) {
    MPI_Init(&argc, &argv);

    int rank, size, value, result;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    // Each process sets its value to its rank 
    value = rank;
    result = 0;

    if (size < 4) {
        std::cout << "At least four processes are needed for this program." << std::endl;
        MPI_Finalize();
        return 1;
    }

    // MPI_Allreduce to send all values to the root process (rank 0) and sum them up
    MPI_Allreduce(&value, &result, 1, MPI_INT, MPI_SUM, MPI_COMM_WORLD);
    
    // Each rank prints out it's own rank and the result of the reduction
    std::cout << "Rank " << rank << " has the result of the reduction: " << result << std::endl;

    MPI_Finalize();
    return 0;
}