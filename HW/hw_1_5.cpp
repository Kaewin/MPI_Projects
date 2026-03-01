#include <iostream>
#include <mpi.h>

int main(int argc, char** argv) {
    
    MPI_Init(&argc, &argv);

    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    if (size < 2) {
        std::cout << "This program requires at least two processes." << std::endl;
        MPI_Finalize();
        return 1;
    }

    // Split communicators:
    int Q = 3; 
    int color1 = rank / Q;  // same color = same row
    MPI_Comm row_comm;
    MPI_Comm_split(MPI_COMM_WORLD, color1, rank, &row_comm);

    int color2 = rank % Q;
    MPI_Comm col_comm;
    MPI_Comm_split(MPI_COMM_WORLD, color2, rank, &col_comm);

    int row_rank, col_rank;
    MPI_Comm_rank(row_comm, &row_rank);
    MPI_Comm_rank(col_comm, &col_rank);

    std::cout << "World rank " << rank
            << " | row " << color1 << " rank " << row_rank
            << " | col " << color2 << " rank " << col_rank
            << std::endl;

    MPI_Finalize();
}