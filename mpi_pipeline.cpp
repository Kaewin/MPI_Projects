#include <mpi.h>
#include <iostream>
#include <cstring>
#include <cstdio>

int main (int argc, char** argv) {
    MPI_Init(&argc, &argv);

    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    if (size != 4) {
        if (rank == 0) std::cerr << "This program requires exactly 4 MPI processes.\n";
        MPI_Finalize();
        return 1;
    }

    if (rank == 0) {
        for (int i = 1; i <= 10; ++i) {
            // Generate a random number to send through the pipeline
            int number = rand() % 100;
            std::cout << "Rank 0 sending number: " << number << std::endl
                        << " to Rank 1" << std::endl;
            MPI_Send(&number, 1, MPI_INT, 1, 0, MPI_COMM_WORLD);
        }
    } else if (rank == 1) {
        for (int i = 0; i < 10; ++i) {
            // Multiply the number by 2
            int number;
            MPI_Recv(&number, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            number *= 2;
            std::cout << "Rank 1 received number, multiplied by 2: " << number << std::endl
                        << " sending to Rank 2" << std::endl;
            MPI_Send(&number, 1, MPI_INT, 2, 0, MPI_COMM_WORLD);
        }
    } else if (rank == 2) {
        for (int i = 0; i < 10; ++i) {
            // Add 5 to the number
            int number;
            MPI_Recv(&number, 1, MPI_INT, 1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            number += 5;
            std::cout << "Rank 2 received number, added 5: " << number
                        << std::endl << " sending to Rank 3" << std::endl;
            MPI_Send(&number, 1, MPI_INT, 3, 0, MPI_COMM_WORLD);
        }
    } else if (rank == 3) {
        for (int i = 0; i < 10; ++i) {
            // Receive the final number
            int number;
            MPI_Recv(&number, 1, MPI_INT, 2, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            std::cout << "Rank 3 received final number: " << number << std::endl;
        }
    }

    MPI_Finalize();
    return 0;
}