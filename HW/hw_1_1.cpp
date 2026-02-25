/*
Write an MPI program (by hand, no LLMs) that passes a message of one integer around in a logical ring of
processes with MPI COMM WORLD. The integer should start at 0 in process 0 and be incremented each time it
passes around the ring, and you should be able to have the message go around the ring N times, where N
is specified at compile time.
*/
#include <iostream>
#include <mpi.h>

int calculate_neighbor(bool direction, int rank, int size) {
    int right_neighbor = (rank + 1) % size;
    int left_neighbor = (rank - 1 + size) % size;
    if (direction == TRUE) {
        return right_neighbor;
    } else {
        return left_neighbor;
    }
}

int main(int argc, char** argv) {

    int N = 5;
    int value = 0;
    int received_value;

    MPI_Init(&argc, &argv);
    
    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	MPI_Comm_size(MPI_COMM_WORLD, &size);

    if (rank < 4) {
        std::cout << "Need at least four processes for this program." << std::endl;
    }

    if (rank == 0) { 
        MPI_Send(
            &value,           // Pointer to data to send
            1,           // Number of elements
            MPI_INT,   // Data type (MPI_INT, MPI_DOUBLE, etc.)
            1,            // Destination rank
            0,             // Message tag (usually 0 for simple cases)
            MPI_COMM_WORLD        // Communicator (usually MPI_COMM_WORLD)
        );
    }

    if (rank == 1) {
        MPI_Recv(
            &received_value,           // Pointer to buffer to receive into
            1,           // Max number of elements to receive
            MPI_INT,   // Data type
            0,          // Source rank (or MPI_ANY_SOURCE)
            MPI_ANY_TAG,             // Message tag (or MPI_ANY_TAG)
            MPI_COMM_WORLD,       // Communicator
            MPI_STATUS_IGNORE // Status object (can use MPI_STATUS_IGNORE)
        );
    }

    std::cout << received_value << std::endl;
    
    MPI_Finalize();
    return 0;
}