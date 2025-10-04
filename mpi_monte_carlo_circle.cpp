#include <mpi.h>
#include <iostream>
#include <random>

int main(int argc, char* argv[]) {
	MPI_Init(&argc, &argv);

	int rank, size;
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	MPI_Comm_size(MPI_COMM_WORLD, &size);
	
	// Each process will do part of the work
	int total_points = 1000000;
	int points_per_process = total_points / size;
	int local_count = 0;

	// Get randomness from hardware
	std::random_device rd;
	// Random number generation algorithm: Mersenne Twister, period 2^19937-1
	// This time we add the rank to it to seed it differently for each process
	std::mt19937 gen(rd() + rank);
	// Generate a probability distribution: a uniform real distribution between 0 and 1
	std::uniform_real_distribution<> dis(0.0, 1.0);

	// Throw random darts
	for(int i = 0; i < points_per_process; i++) {
		double x = dis(gen);
		double y = dis(gen);

		// Check if inside circle
		// Using quarter of a unit circle
		if(x*x + y*y <= 1.0) {
			local_count++;
		}
	}

	// Synchronize the processes
	MPI_Barrier(MPI_COMM_WORLD);

	// Gather results to rank 0
	if (rank == 0) {
		int total_in_circle = local_count;

		for(int i = 1; i < size; i++) {
			int received_count;
			MPI_Recv(&received_count, 1, MPI_INT, i, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
			total_in_circle += received_count;
		}

	// Multiply by 4 to isolate PI, given first quadrant is pi/4;
	double pi_estimate = 4.0 * total_in_circle / total_points;
	std::cout << "PI Estimate with: " << size << " processes: " << pi_estimate << std::endl;

	} else {
			MPI_Send(&local_count, 1, MPI_INT, 0, 0, MPI_COMM_WORLD);
		}

	MPI_Finalize();

	return 0;
	}


