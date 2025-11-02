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
    
    if (size != 4) {  // <<< 4 processes now
        if (rank == 0) {
            std::cerr << "Error: This program requires exactly 4 processes!" << std::endl;
            std::cerr << "Usage: mpirun -n 4 ./program" << std::endl;
        }
        MPI_Finalize();
        return 1;
    }
    
    MPI_Win win;
    int data[50] = {0};
    
    {
        MPI_Group world_group;
        MPI_Comm_group(MPI_COMM_WORLD, &world_group);
        
        // TARGETS: ranks 2 and 3 <
        if (rank == 2 || rank == 3) {  // <<< Both rank 2 AND rank 3 are targets
            MPI_Group origin_group;
            int origin_ranks[2] = {0, 1};  // Both origins access both targets
            
            MPI_Group_incl(world_group, 2, origin_ranks, &origin_group);
            
            int err;
            err = MPI_Win_create(&data, sizeof(int) * 50, sizeof(int),
                                MPI_INFO_NULL, MPI_COMM_WORLD, &win);
            check_mpi_error(err, "MPI_Win_create");
            
            std::cout << "Target " << rank << " BEFORE: ";  // <<< Show which target
            for (int i = 0; i < 50; i++) std::cout << data[i] << " ";
            std::cout << std::endl;
            
            MPI_Win_post(origin_group, 0, win);
            
            err = MPI_Win_wait(win);
            check_mpi_error(err, "MPI_Win_wait");
            
            std::cout << "Target " << rank << " AFTER: ";  // <<< Show which target
            for (int i = 0; i < 50; i++) std::cout << data[i] << " ";
            std::cout << std::endl;
            
            MPI_Group_free(&origin_group);
            
        // ORIGINS: ranks 0 and 1 <
        } else {  // rank 0 or rank 1
            MPI_Group target_group;
            int target_ranks[2] = {2, 3};  // <<< Access BOTH targets (ranks 2 and 3)
            
            MPI_Group_incl(world_group, 2, target_ranks, &target_group);  // <<< 2 targets
            
            int err;
            err = MPI_Win_create(&data, sizeof(int) * 50, sizeof(int),
                                MPI_INFO_NULL, MPI_COMM_WORLD, &win);
            check_mpi_error(err, "MPI_Win_create");
            
            MPI_Win_start(target_group, 0, win);
            
            // Each origin creates different data
            int send_data[25];
            if (rank == 0) {
                for (int i = 0; i < 25; i++) {
                    send_data[i] = i + 1;  // Origin 0: 1-25
                }
            } else {
                for (int i = 0; i < 25; i++) {
                    send_data[i] = i + 26;  // Origin 1: 26-50
                }
            }
            
            int offset = (rank == 0) ? 0 : 25;
            
            // <<< Write to BOTH targets (loop through target ranks)
            for (int target : {2, 3}) {  // <<< C++11 range-based for loop
                err = MPI_Put(send_data, 25, MPI_INT,
                             target,    // <<< Write to this target
                             offset,
                             25, MPI_INT, win);
                check_mpi_error(err, "MPI_Put");
            }
            
            MPI_Win_complete(win);
            
            MPI_Group_free(&target_group);
        }
        
        MPI_Group_free(&world_group);
    }
    
    MPI_Win_free(&win);
    MPI_Finalize();
}