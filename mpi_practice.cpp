#include <mpi.h>
#include <iostream>

int main(int argc, char* argv[]) {

	int rank, size;

	MPI_Init(&argc, &argv);

	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	MPI_Comm_size(MPI_COMM_WORLD, &size);

	if (rank == 2) {
		double myarray[50];

		std::cout << "Process 2 sending array" << std::endl;

		for(int i = 0; i < 50; i++) {
			myarray[i] = i * 2;
		}

		MPI_Send(myarray, 50, MPI_DOUBLE, 0, 0, MPI_COMM_WORLD);
	}

	if (rank == 0) {
		double receivedarray[50];

		std::cout << "Process 0 receiving array" << std::endl;

		MPI_Recv(receivedarray, 50, MPI_DOUBLE, 2, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

		for(auto number : receivedarray) {
			std::cout << number << std::endl;
		}
	}

	MPI_Finalize();

	return 0;
}