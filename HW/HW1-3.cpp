/*
Problem 3 (15pts/12pts):
Write a parallel vector-vector element by element multiplication program (by hand, no LLM) followed by
a tree reduction to compute the dot-product of two vectors of length N with P processes using MPI. 

For instance, show with N = 1024, 2048, 4096 and P = 1, 2, 4.
Use only point-to-point operations. 
Code your own tree reduction using sends and receives.
Divide the data as equally as you can between your processes. We will discuss how to compute these
partitions in class early next week (week of February 16).

Use MPI Wtime() to measure the cost of the operation as you vary P and N . We will explain how to use
this operation in lecture.

Use easy-to-evaluate data for testing in the vector elements for correctness testing (but not all zeroes).
*/
#include <iostream>
#include <mpi.h>
#include <vector>

using namespace std;

int scalar_product(vector<int> a, vector<int> b, int start, int end) {

    int product = 0;
    for(int i = start; i < end; i++)
        product += (a[i]*b[i]);
    return product;
}

int main(int argc, char** argv) {

    MPI_Init(&argc, &argv);

    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    int N = 4096;
    std::vector<int> a(N, 1);
    std::vector<int> b(N, 1);

    if (size > 4 || size == 3) {
        std::cout << "Need 1, 2, or 4 processes for this program." << std::endl;
        return 1;
    }

    // N/p gives the number of vector elements each process will receive
    int process_elements = N/size;
    int starting_point = rank * process_elements;
    int ending_point = starting_point + process_elements;

    double start_time = MPI_Wtime();

    int result = scalar_product(a, b, starting_point, ending_point);

    int step = 1;

    while (step < size) {
        if (rank % (2 * step) == 0) {
            // I'm a receiver
            int received_value;
            int partner = rank + step;

            if (partner < size) { // Make sure partner exists
                MPI_Recv(
                    &received_value,           // Pointer to buffer to receive into
                    1,           // Max number of elements to receive
                    MPI_INT,   // Data type
                    partner,          // Source rank (or MPI_ANY_SOURCE)
                    MPI_ANY_TAG,             // Message tag (or MPI_ANY_TAG)
                    MPI_COMM_WORLD,       // Communicator
                    MPI_STATUS_IGNORE // Status object (can use MPI_STATUS_IGNORE)
                );   
                result += received_value;
            }
        } else if (rank % (2 * step) == step) {
            // I'm a sender
            int partner = rank - step;
            MPI_Send(
                &result,           // Pointer to data to send
                1,           // Number of elements
                MPI_INT,   // Data type (MPI_INT, MPI_DOUBLE, etc.)
                partner,            // Destination rank
                0,             // Message tag (usually 0 for simple cases)
                MPI_COMM_WORLD        // Communicator (usually MPI_COMM_WORLD)
            );    
            break;
        }
        step *= 2;
    }

    if (rank == 0) {
        cout << "============================" << endl;
        cout << "Final dot product: " << result << endl;
        double end_time = MPI_Wtime();
        double local_elapsed = end_time - start_time;
        cout << "Time elapsed: " << local_elapsed << endl;
        cout << "============================" << endl;
        cout << endl;
    }

    cout << "Result of scalar product: " << result << endl;
    cout << "Number of processes: " << size << endl;
    cout << "Each element got: " << process_elements << " elements." << endl;
    cout << endl;

    MPI_Finalize();
    return 0;
}