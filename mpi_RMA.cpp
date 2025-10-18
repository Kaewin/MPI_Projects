#include <iostream>
#include <mpi.h>

int main(int argc, char** argv) {
	MPI_Init(&argc, &argv);

	int rank, size;
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	MPI_Comm_size(MPI_COMM_WORLD, &size);

	{
		MPI_Win win;

		int data[5] = {0,0,0,0,0};

		MPI_Win_create(&array1,
						)
	}

	MPI_Finalize();
}