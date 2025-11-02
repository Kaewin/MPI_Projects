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
    
    // MODIFIED: Changed to require 3 processes <
    if (size != 3) {
        if (rank == 0) {
            std::cerr << "Error: This program requires exactly 3 processes!" << std::endl;
            std::cerr << "Usage: mpirun -n 3 ./program" << std::endl;
        }
        MPI_Finalize();
        return 1;
    }
    
    MPI_Win win;
    int data[50] = {0};
    
    {
        MPI_Group world_group;
        MPI_Comm_group(MPI_COMM_WORLD, &world_group);
        
        // MODIFIED: Target is now rank 2 <
        if (rank == 2) {
            MPI_Group origin_group;
            
            // MODIFIED: Group now contains TWO origins (ranks 0 and 1) <
            int origin_ranks[2] = {0, 1};  // Both rank 0 and rank 1
            
            // MODIFIED: Count is now 2 (two origins) <
            MPI_Group_incl(world_group,
                        2,                // Two origins now! <
                        origin_ranks,
                        &origin_group);
            
            int err;
            err = MPI_Win_create(&data, sizeof(int) * 50, sizeof(int),
                                MPI_INFO_NULL, MPI_COMM_WORLD, &win);
            check_mpi_error(err, "MPI_Win_create");
            
            std::cout << "Target BEFORE: ";
            for (int i = 0; i < 50; i++) std::cout << data[i] << " ";
            std::cout << std::endl;
            
            MPI_Win_post(origin_group, 0, win);
            
            err = MPI_Win_wait(win);
            check_mpi_error(err, "MPI_Win_wait");
            
            std::cout << "Target AFTER: ";
            for (int i = 0; i < 50; i++) std::cout << data[i] << " ";
            std::cout << std::endl;
            
            MPI_Group_free(&origin_group);
            
        // MODIFIED: Origins are now ranks 0 and 1 (both use same code structure) <
        } else {
            MPI_Group target_group;
            
            // MODIFIED: Target is now rank 2 <
            int target_ranks[1] = {2};  // Target is rank 2 now
            
            MPI_Group_incl(world_group,
                        1,
                        target_ranks,
                        &target_group);
            
            int err;
            err = MPI_Win_create(&data, sizeof(int) * 50, sizeof(int),
                                MPI_INFO_NULL, MPI_COMM_WORLD, &win);
            check_mpi_error(err, "MPI_Win_create");
            
            MPI_Win_start(target_group, 0, win);
            
            // NEW: Each origin writes to different half of the array <
            int send_data[25];  // Only 25 elements per origin now <
            
            // NEW: Different data for each origin <
            if (rank == 0) {
                // Origin 0: writes values 1-25
                for (int i = 0; i < 25; i++) {
                    send_data[i] = i + 1;
                }
            } else {
                // Origin 1: writes values 26-50
                for (int i = 0; i < 25; i++) {
                    send_data[i] = i + 26;
                }
            }
            
            // NEW: Different offset for each origin <
            int offset = (rank == 0) ? 0 : 25;  // Origin 0: offset 0, Origin 1: offset 25 <
            
            // MODIFIED: Put 25 elements at different offsets <
            err = MPI_Put(send_data, 25, MPI_INT,  // 25 elements now <
                         2,          // Target is rank 2 <
                         offset,     // Different offset per origin <
                         25, MPI_INT, win);
            check_mpi_error(err, "MPI_Put");
            
            MPI_Win_complete(win);
            
            MPI_Group_free(&target_group);
        }
        
        MPI_Group_free(&world_group);
    }
    
    MPI_Win_free(&win);
    MPI_Finalize();
}