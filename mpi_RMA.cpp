#include <iostream>
#include <mpi.h>

void check_mpi_error(int err, const char* function_name) {
    if (err != MPI_SUCCESS) {
        char error_string[MPI_MAX_ERROR_STRING];
        int length;
        MPI_Error_string(err, error_string, &length);
        std::cerr << "Error in " << function_name << ": " 
                  << error_string << std::endl;
        MPI_Abort(MPI_COMM_WORLD, err);
    }
}

int main(int argc, char** argv) {
	MPI_Init(&argc, &argv);

	int rank, size;
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	MPI_Comm_size(MPI_COMM_WORLD, &size);

	// Add this validation:
	if (size != 2) {
		if (rank == 0) {
			std::cerr << "Error: This program requires exactly 2 processes!" << std::endl;
			std::cerr << "Usage: mpirun -n 2 ./program" << std::endl;
		}
		MPI_Finalize();
		return 1;
	}

	MPI_Win win;
	// int data[5] = {0,0,0,0,0};
	int data[50] = {0};
	{
		MPI_Group world_group;
		MPI_Comm_group(MPI_COMM_WORLD, &world_group);

		if (rank == 1) { 
			MPI_Group origin_group;
			int origin_ranks[1] = {0};  // Array containing rank 0
			
			MPI_Group_incl(world_group,     // Source group
						1,                // How many ranks
						origin_ranks,     // Array of ranks
						&origin_group);   // Output group

			// MPI_Win_create(&data,
			// 				sizeof(int) * 5,
			// 				sizeof(int),
			// 				MPI_INFO_NULL,
			// 				MPI_COMM_WORLD,
			// 				&win);

			int err;

			err = MPI_Win_create(&data, sizeof(int) * 50, sizeof(int),
									MPI_INFO_NULL, MPI_COMM_WORLD, &win);
			check_mpi_error(err, "MPI_Win_create");

			std::cout << "Target BEFORE: ";
			for (int i = 0; i < 50; i++) std::cout << data[i] << " ";
			std::cout << std::endl;

			// PSCW pattern
			MPI_Win_post(origin_group, 0, win);

			// wait for origin to finish 
			// MPI_Win_wait(win);

			err = MPI_Win_wait(win);
			check_mpi_error(err, "MPI_Win_wait");

			std::cout << "Target AFTER: ";
			for (int i = 0; i < 50; i++) std::cout << data[i] << " ";
			std::cout << std::endl;

			// Cleanup
			MPI_Group_free(&origin_group);		
		} else {
			MPI_Group target_group;
			int target_ranks[1] = {1};  // Array containing rank 1
			
			MPI_Group_incl(world_group,     // Source group
						1,                // How many ranks
						target_ranks,     // Array of ranks
						&target_group);   // Output group

			// MPI_Win_create(nullptr,
			// 				0,
			// 				1,
			// 				MPI_INFO_NULL,
			// 				MPI_COMM_WORLD,
			// 				&win);

			int err;

			err = MPI_Win_create(&data, sizeof(int) * 50, sizeof(int),
									MPI_INFO_NULL, MPI_COMM_WORLD, &win);
			check_mpi_error(err, "MPI_Win_create");


			// PSCW pattern
			MPI_Win_start(target_group, 0, win);

			int send_data[50];
			for (int i = 0; i < 50; i++) {
				send_data[i] = i + 1;
			}

			// MPI_Put(send_data,           // Source data
			// 		5,                    // Count
			// 		MPI_INT,              // Datatype
			// 		1,                    // Target rank
			// 		0,                    // Offset in target window
			// 		5,                    // Count on target
			// 		MPI_INT,              // Target datatype
			// 		win);                 // Window

			err = MPI_Put(send_data, 50, MPI_INT, 1, 0, 50, MPI_INT, win);
			check_mpi_error(err, "MPI_Put");

			MPI_Win_complete(win);
			// Cleanup
			MPI_Group_free(&target_group);
		}
		// Both processes: cleanup
		MPI_Group_free(&world_group);
	}
	MPI_Win_free(&win);
	MPI_Finalize();
}