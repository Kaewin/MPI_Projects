#include <mpi.h>
#include <iostream>
#include <random>
#include <vector>  
#include <cstdlib> 

int main(int argc, char* argv[]) {
	MPI_Init(&argc, &argv);

	int rank, size;
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	MPI_Comm_size(MPI_COMM_WORLD, &size);

	if(argc != 2) {
		if(rank == 0) {
			std::cerr << "Usage: mpirun -n <processes>" << argv[0] << " <number of points>" << std::endl;
		}
		MPI_Finalize();
		return 1;
	}

	long long total_points = std::atoll(argv[1]);

	long long base_points = total_points / size;
	long long remainder = total_points % size;
	long long points_per_process = base_points;

	if(rank < remainder) { 
		points_per_process++;
	}

	double start_time = MPI_Wtime();

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

	MPI_Barrier(MPI_COMM_WORLD);

	// Non-blocking code
	long long total_in_circle = 0;
	long long received_count = 0;
	if (rank == 0) {
		total_in_circle = local_count;  
		
		std::vector<long long> received_counts(size-1);  
		std::vector<MPI_Request> requests(size-1);       
		
		for(int i = 1; i < size; i++) {
			MPI_Irecv(&received_counts[i-1], 1, MPI_LONG_LONG, i, 0, MPI_COMM_WORLD, &requests[i-1]);
		}

		MPI_Waitall(size - 1, requests.data(), MPI_STATUSES_IGNORE);

		for(int i = 0; i < size-1; i++) {
			total_in_circle += received_counts[i];
		}

		double pi_estimate = 4.0 * total_in_circle / total_points;
	} else {
		MPI_Request send_request;
		MPI_Isend(&local_count, 1, MPI_LONG_LONG, 0, 0, MPI_COMM_WORLD, &send_request);

		MPI_Wait(&send_request, MPI_STATUS_IGNORE);
	}

	double end_time = MPI_Wtime();
	double local_elapsed = end_time - start_time;

	double total_time = 0;
	MPI_Reduce(&local_elapsed, &total_time, 1, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);

	if(rank == 0) {
		double avg_time = total_time / size;
		double pi_estimate = 4.0 * total_in_circle / total_points;

		std::cout << "Points: " << total_points << std::endl;
		std::cout << "Processes: " << size << std::endl;
		std::cout << "PI estimate: " << pi_estimate << std::endl;
		std::cout << "Average time per rank: " << avg_time << " seconds" << std::endl;
	}

	MPI_Finalize();
	return 0;
}