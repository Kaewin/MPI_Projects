#include <mpi.h>
#include <iostream>

int main(int arc, char& argv[]) {
	MPI_Init(&argc, &argv);

	int rank, size;
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	MPI_Comm_size(MPI_COMM_WORLD, &size);

	// Same as the mpi_monte_carlo_circle.cpp
	int total_points = 1000000;
	int points_per_process = total_points / size;
	int local_count = 0;
	std::random_device rd;
	std::mt19937 gen(rd() + rank);
	std::uniform_real_distribution<> dis(0.0, 1.0);

	// Throw random darts
	for(int i = 0; i < points_per_process; i++) {
		double x = dis(gen);
		double y = dis(gen);
		if(x*x + y*y <= 1.0) {
			local_count++;
		}
	}

	// Non-blocking communication
	if (rank == 0) {
		int total_in_circle = local_count;

		// Array to store received values
		std::vector<int> received_counts(size-1);
		// Array of request handles
		std::vector<MPI_Request> requests(size-1);

		// Start all receives
		for (int i = 1; i < size; i++) {
			MPI_Irecv(&received_counts[i-1], 1, MPI_INT, i, 0, MPI_COMM_WORLD, &requests[i-1]);
		}

		std::cout << "Rank 0: Started all receives, waiting for results: " << std::endl;

		// Wait for all receives to complete
		MPI_Waitall(size - 1, requests.data(), MPI_STATUSES_IGNORE);

		// Sum up all counts
		for(int i = 0; i < size-1; i++) {
			total_in_circle += received_counts[i];
		}

		double pi_estimate = 4.0 * total_in_circle / total_points;
		std::cout << "PI estimate with " << size << " processes: " << pi_estimate << std::endl;
	} else {
		// Non-blocking send
		MPI_Request send_request;
		MPI_Isend(&local_count, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, &send_request);

		std::cout << "Rank: " << rank << ": sent my result:" << std::endl;

		// Make sure send completes before exiting
		MPI_Wait(&send_request, MPI_STATUS_IGNORE);
	}

	MPI_Finalize();
	return 0;
}