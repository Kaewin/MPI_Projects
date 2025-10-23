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
    
    if (size != 3) {
        if (rank == 0) {
            std::cerr << "Error: This program requires exactly 3 processes!" << std::endl;
            std::cerr << "Usage: mpirun -n 3 ./program" << std::endl;
        }
        MPI_Finalize();
        return 1;
    }
    
    MPI_Win win;
    int *data = nullptr;
    
    if (rank == 2) {  // TARGET - now rank 2
        // CRITICAL: Use MPI_Alloc_mem for passive target!
        int err = MPI_Alloc_mem(50 * sizeof(int), MPI_INFO_NULL, &data);
        check_mpi_error(err, "MPI_Alloc_mem");
        
        // // Initialize to zeros
        // for (int i = 0; i < 50; i++) {
        //     data[i] = 0;
        // }

		// NEW: Initialize with values 100-149 (so origins have something to read)
		for (int i = 0; i < 50; i++) {
			data[i] = i + 100;  // 100, 101, 102, ..., 149
		}
        
        // Create window
        err = MPI_Win_create(data, 50 * sizeof(int), sizeof(int),
                            MPI_INFO_NULL, MPI_COMM_WORLD, &win);
        check_mpi_error(err, "MPI_Win_create");
        
		std::cout << "Target has data ready: 100 to 149" << std::endl;
        
        // TARGET DOES NOTHING HERE! Passive!
        
        // Barrier to wait for origin to finish (for demo purposes)
        MPI_Barrier(MPI_COMM_WORLD);
        
        // Cleanup
        MPI_Win_free(&win);
        MPI_Free_mem(data);
        
    } else {  // ORIGIN (ranks 0 and 1)
        // Origin creates dummy window
        int dummy[1];
        int err = MPI_Win_create(dummy, sizeof(int), sizeof(int),
                                MPI_INFO_NULL, MPI_COMM_WORLD, &win);
        check_mpi_error(err, "MPI_Win_create");
        
        // Lock target (rank 2) with SHARED lock
        err = MPI_Win_lock(MPI_LOCK_SHARED, 2, 0, win);
        check_mpi_error(err, "MPI_Win_lock");
        
		// Prepare receive buffer
		int receive_data[50];
		
		// Read from target
		MPI_Get(receive_data,  // ← Destination (local)
				50,             // Count
				MPI_INT, 
				2,              // ← Source (target rank 2)
				0,              // Offset
				50, 
				MPI_INT, 
				win);		
        
		// Unlock target
		MPI_Win_unlock(2, win);  // ← Target rank 2
		
		// Print what we got (just first 10 values)
		std::cout << "Origin " << rank << " read first 10 values: ";
		for (int i = 0; i < 10; i++) {
			std::cout << receive_data[i] << " ";
		}
		std::cout << "..." << std::endl;
        
        // Barrier
        MPI_Barrier(MPI_COMM_WORLD);
        
        // Cleanup
        MPI_Win_free(&win);
    }
    
    MPI_Finalize();
    return 0;
}	