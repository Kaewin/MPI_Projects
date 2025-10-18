#include <iostream>
#include <mpi.h>

int main(int argc, char** argv) {
	MPI_Init(&argc, &argv);

	int rank, size;
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	MPI_Comm_size(MPI_COMM_WORLD, &size);

	MPI_Win win;
	int data[5] = {0,0,0,0,0};
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

			MPI_Win_create(&data,
							sizeof(int) * 5,
							sizeof(int),
							MPI_INFO_NULL,
							MPI_COMM_WORLD,
							&win);

			std::cout << "Target BEFORE: ";
			for (int i = 0; i < 5; i++) std::cout << data[i] << " ";
			std::cout << std::endl;

			// PSCW pattern
			MPI_Win_post(origin_group, 0, win);
			// ... wait for origin to finish ...
			MPI_Win_wait(win);

			std::cout << "Target AFTER: ";
			for (int i = 0; i < 5; i++) std::cout << data[i] << " ";
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

			MPI_Win_create(nullptr,
							0,
							0,
							MPI_INFO_NULL,
							MPI_COMM_WORLD,
							&win);
			// PSCW pattern
			MPI_Win_start(target_group, 0, win);

			int send_data[5] = {1,2,3,4,5};

			MPI_Put(send_data,           // Source data
					5,                    // Count
					MPI_INT,              // Datatype
					1,                    // Target rank
					0,                    // Offset in target window
					5,                    // Count on target
					MPI_INT,              // Target datatype
					win);                 // Window

			MPI_Win_complete(win);
			// Cleanup
			MPI_Group_free(&target_group);
		}
		// Both processes: cleanup
		MPI_Group_free(&world_group);
	}
	MPI_Finalize();
}