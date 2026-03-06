#include <iostream>
#include <mpi.h>
#include <vector>
#include <algorithm>

int main(int argc, char**argv) {
    MPI_Init(&argc, &argv);
    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    if (size < 2) {
        if (rank == 0) std::cout << "This program needs at least two processes to run." << std::endl;
        MPI_Finalize();
        return 1;
    }

    int n = 16;
    std::vector<int> array(n, -1);

    for(int i = 0; i < n; i++) {
        array[i] = i;
        // std::cout << "Array element " << i << " contains " << array[i] << std::endl;
    }

    int local_n = n / size;
    int start = rank * local_n;
    std::vector<int> local_array(local_n + 2, -1);

    // Modern way to copy an array
    std::copy(array.begin() + start, array.begin() + start + local_n, local_array.begin() + 1);

    int next = (rank + 1) % size;
    int prev = (rank - 1 + size) % size;

    // Modern way to loop through an array
    for (int val : local_array) {
        std::cout << val << " ";
    }
    std::cout << std::endl;
    std::cout << std::endl;

    MPI_Barrier(MPI_COMM_WORLD);

    MPI_Sendrecv(&local_array[local_n], 1, MPI_INT, next, 0,
                &local_array[0],       1, MPI_INT, prev, 0,
                MPI_COMM_WORLD, MPI_STATUS_IGNORE);

    MPI_Sendrecv(&local_array[1],         1, MPI_INT, prev, 0,
                &local_array[local_n+1], 1, MPI_INT, next, 0,
                MPI_COMM_WORLD, MPI_STATUS_IGNORE);

    MPI_Barrier(MPI_COMM_WORLD);
    std::cout << "Rank " << rank <<": ";
    // Modern way to loop through an array
    for (int val : local_array) {
        std::cout << val << " ";
    }
    std::cout << std::endl;
    std::cout << std::endl;

    MPI_Finalize();
    return 0;
}