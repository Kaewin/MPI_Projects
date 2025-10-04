#include <mpi.h>
#include <iostream>
#include <random>

int main(int argc, char* argv[]) {
	MPI_Init(&argc, &argv);

	int rank, size;
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	MPI_Comm_size(MPI_COMM_WORLD, &size);
	
	// In this verison we are going to take in the number of points 
	// From the command line 

	// Old code:
	// int total_points = 1000000;
	// int points_per_process = total_points / size;
	// int local_count = 0;

	if(argc != 2) {
		if(rank == 0) {
			std::cerr << "Usage: mpirun -n <processes>" << argv[0] << " <number of points>" << std::endl;
		}
		MPI_Finalize();
		return 1;
	}

	// Use to convert a command line string to a long long 
	long long total_points = std::atoll(argv[1]);

	// Distribute points
	long long base_points = total_points / size;
	long long remainder = total_points % size;
	long long points_per_process = base_points;

	if(rank < remainder) { 
		points_per_process++;
	}

	// Start timer
	double start_time = MPI_Wtime();

	// Monte-Carlo Computation
	long long local_count = 0;
	std::random_device rd;
	std::mt19937 gen(rd() ^ (rank * 123456));
	std::uniform_real_distribution<> dis(0.0, 1.0);

	for(int i = 0; i < points_per_process; i++) {
		double x = dis(gen);
		double y = dis(gen);
		if(x*x + y*y <= 1.0) {
			local_count++;
		}
	}

	// Synchronize the processes
	MPI_Barrier(MPI_COMM_WORLD);

	// Gather results to rank 0
	long long total_in_circle = 0;
	if (rank == 0) {
		total_in_circle = local_count;
		for(int i = 1; i < size; i++) {
			long long received_count;
			MPI_Recv(&received_count, 1, MPI_LONG_LONG, i, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
			total_in_circle += received_count;
		}
	} else {
			MPI_Send(&local_count, 1, MPI_LONG_LONG, 0, 0, MPI_COMM_WORLD);
	}

	// End timing
	double end_time = MPI_Wtime();
	double local_elapsed = end_time - start_time;

	// Gather all times to rank 0 to compute average
	double total_time = 0;
	MPI_Reduce(&local_elapsed, &total_time, 1, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);

	if(rank == 0) {
		double avg_time = total_time / size;
		double pi_estimate = 4.0 * total_in_circle / total_points;

		std::cout << "Points: " << total_points << std::endl;
		std::cout << "Processes: " << size << std::endl;
		std::cout << "PI estimate: " << pi_estimate << std::endl;
		std::cout << "Average time per rank: " << avg_time << " seconds" << std::endl;
		std::cout << "Total time rank 0: " << local_elapsed << " seconds" << std::endl; 		
	}

	MPI_Finalize();
	return 0;
}