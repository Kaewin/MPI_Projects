#include <mpi.h>
#include <iostream>
#include <cstring>
#include <cstdio>

int main(int argc, char** argv) {
    MPI_Init(&argc, &argv);

    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    if (size != 2) {
        if (rank == 0) std::cerr << "This program requires only 2 MPI Processes.\n";
        MPI_Finalize();
        return 1;
    }

    const int MAX_MSG = 256;
    char buffer[MAX_MSG];
    const int TAG = 0;
    const int iterations = 10;

    if (rank == 0) {
        for (int i = 0; i < iterations; ++i) {
            // std::snprintf formats text into the provided C buffer.
            // Signature: int snprintf(char* str, std::size_t size, const char* format, ...);
            // It writes at most (size-1) characters plus the terminating '\0', preventing buffer overflow
            // when used correctly. It returns the number of characters that would have been written
            // (not including the terminating '\0') or a negative value on encoding error.
            // We use the std:: version from <cstdio> (placed in namespace std).
            std::snprintf(buffer, MAX_MSG, "Ping %d from rank 0", i);
            MPI_Send(buffer, (int)std::strlen(buffer) + 1, MPI_CHAR, 1, TAG, MPI_COMM_WORLD);

            MPI_Recv(buffer, MAX_MSG, MPI_CHAR, 1, TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            std::cout << "Rank 0 received: " << buffer << std::endl;
        }
    } else if (rank == 1) {
        for (int i = 0; i < iterations; ++i) {
            MPI_Recv(buffer, MAX_MSG, MPI_CHAR, 0, TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            std::cout << "Rank 1 received: " << buffer << std::endl;

            // Same notes as above: snprintf safely formats into 'buffer'.
            // If you prefer modern C++ alternatives, consider std::string with std::format (C++20)
            // or using std::ostringstream for portability.
            std::snprintf(buffer, MAX_MSG, "Pong %d from rank 1", i);
            MPI_Send(buffer, (int)std::strlen(buffer) + 1, MPI_CHAR, 0, TAG, MPI_COMM_WORLD);
        }
    }

    MPI_Finalize();
    return 0;
}