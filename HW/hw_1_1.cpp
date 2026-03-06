/*
Write an MPI program (by hand, no LLMs) that passes a message of one integer around in a logical ring of
processes with MPI COMM WORLD. The integer should start at 0 in process 0 and be incremented each time it
passes around the ring, and you should be able to have the message go around the ring N times, where N
is specified at compile time.
*/
#include <iostream>
#include <mpi.h>

int main(int argc, char** argv) {

    int N = 5;
    int value = 0;

    MPI_Init(&argc, &argv);
    
    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	MPI_Comm_size(MPI_COMM_WORLD, &size);

    if (size < 2) {
        std::cout << "Need at least two processes for this program." << std::endl;
        return 1;
    }

    int next_rank = (rank + 1) % size; // Next rank in the ring
    int prev_rank = (rank - 1 + size) % size; // Previous rank in the ring

    if (rank == 0) {
    value = 0;
    
        for (int i = 0; i < N; i++) {
            std::cout << "Rank " << rank << " is starting with value " << value << std::endl;
            MPI_Send(
                &value,           // Pointer to data to send
                1,           // Number of elements
                MPI_INT,   // Data type (MPI_INT, MPI_DOUBLE, etc.)
                next_rank,            // Destination rank
                0,             // Message tag (usually 0 for simple cases)
                MPI_COMM_WORLD        // Communicator (usually MPI_COMM_WORLD)
            );            
            MPI_Recv(
                &value,           // Pointer to buffer to receive into
                1,           // Max number of elements to receive
                MPI_INT,   // Data type
                prev_rank,          // Source rank (or MPI_ANY_SOURCE)
                MPI_ANY_TAG,             // Message tag (or MPI_ANY_TAG)
                MPI_COMM_WORLD,       // Communicator
                MPI_STATUS_IGNORE // Status object (can use MPI_STATUS_IGNORE)
            );            
            value++;
        }   
        std::cout << "Rank " << rank << " received value " << value << " from process " << prev_rank << std::endl;
        
    } else {
        for (int i = 0; i < N; i++) {
            MPI_Recv(
                &value,           // Pointer to buffer to receive into
                1,           // Max number of elements to receive
                MPI_INT,   // Data type
                prev_rank,          // Source rank (or MPI_ANY_SOURCE)
                MPI_ANY_TAG,             // Message tag (or MPI_ANY_TAG)
                MPI_COMM_WORLD,       // Communicator
                MPI_STATUS_IGNORE // Status object (can use MPI_STATUS_IGNORE)
            ); 
            value++;
            MPI_Send(
                &value,           // Pointer to data to send
                1,           // Number of elements
                MPI_INT,   // Data type (MPI_INT, MPI_DOUBLE, etc.)
                next_rank,            // Destination rank
                0,             // Message tag (usually 0 for simple cases)
                MPI_COMM_WORLD        // Communicator (usually MPI_COMM_WORLD)
            );
        }
    }
    MPI_Finalize();
    return 0;
}