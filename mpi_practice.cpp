#include <mpi.h>
#include <iostream>

int main(int argc, char* argv[]) {
	MPI_Init(&argc, &argv);

	int rank, size;
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	MPI_Comm_size(MPI_COMM_WORLD, &size);

	double start = MPI_Wtime();

	for(int i = 0; i < 1000000; i++) {
        double x = i * 2.5;
    }

	double end = MPI_Wtime();
	double elapsed = end - start;

	std::cout << "Process " << rank << " took " << elapsed << " seconds" << std::endl << std::endl;;

	MPI_Barrier(MPI_COMM_WORLD);

	double received = 0;
	MPI_Reduce(&elapsed, &received, 1, MPI_DOUBLE, MPI_MIN, 0, MPI_COMM_WORLD);

	if (rank == 0) {
		std::cout << "Shortest process: " << received << std::endl;
	}

	MPI_Finalize();
	return 0;
}