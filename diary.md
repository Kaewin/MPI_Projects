# 10/18/2025

Going through Nicole's RMA notes. 

## MPI Template Code:

### MPI Initialization

Standard MPI initialization.

```cpp
int main(int argc, char** argv) {
	MPI_Init(&argc, &argv);

	int rank, size;
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	MPI_Comm_size(MPI_COMM_WORLD, &size);

	{
		// Do stuff
	}

	MPI_Finalize();
}
```

### Get the world group
```cpp
MPI_Group world_group;
MPI_Comm_group(MPI_COMM_WORLD, &world_group);
```

This creates a group containing all processes in MPI_COMM_WORLD.

### Making Specific Group

For Target (rank 1) - needs group with origin (rank 0):
```cpp
if (rank == 1) {
    MPI_Group origin_group;
    int origin_ranks[1] = {0};  // Array containing rank 0
    
    MPI_Group_incl(world_group,     // Source group
                   1,                // How many ranks
                   origin_ranks,     // Array of ranks
                   &origin_group);   // Output group
}
```

For Origin (rank 0) - needs group with target (rank 1):
```cpp
if (rank == 0) {
    MPI_Group target_group;
    int target_ranks[1] = {1};  // Array containing rank 1
    
    MPI_Group_incl(world_group,     // Source group
                   1,                // How many ranks
                   target_ranks,     // Array of ranks
                   &target_group);   // Output group
}
```
## Data Declaration

Declare the data array that will be transferred. Must be declared **before** window creation and remain in scope throughout PSCW operations.

```cpp
int data[5] = {0, 0, 0, 0, 0};  // Target's memory that will be accessed
MPI_Win win;                     // Window object for RMA
```

## Window Creation

Both processes must call `MPI_Win_create` because it's a **collective operation**.

### Target (rank 1) - exposes actual memory:

```cpp
MPI_Win_create(&data,              // Pointer to memory to expose
               sizeof(int) * 5,    // Size in bytes
               sizeof(int),        // Displacement unit (for addressing)
               MPI_INFO_NULL,      // Info hints (none)
               MPI_COMM_WORLD,     // Communicator
               &win);              // Output: window object
```

### Origin (rank 0) - participates but doesn't expose:

```cpp
int dummy[1];  // Dummy array
MPI_Win_create(dummy,              // Dummy pointer
               sizeof(int),        // Dummy size
               sizeof(int),        // Must be > 0 (use 1 or sizeof(int))
               MPI_INFO_NULL,      // Info hints (none)
               MPI_COMM_WORLD,     // Communicator
               &win);              // Output: window object
```

---

## PSCW Pattern - Target Process (rank 1)

The target **exposes** its memory and waits for origins to finish.

```cpp
if (rank == 1) {
    // Print data before
    std::cout << "Target BEFORE: ";
    for (int i = 0; i < 5; i++) std::cout << data[i] << " ";
    std::cout << std::endl;
    
    // Start exposure epoch
    MPI_Win_post(origin_group,  // Group of origins that can access
                 0,              // Assert flags (0 = none)
                 win);           // Window object
    
    // Wait for all origins to complete
    MPI_Win_wait(win);           // Closes exposure epoch
    
    // Print data after
    std::cout << "Target AFTER: ";
    for (int i = 0; i < 5; i++) std::cout << data[i] << " ";
    std::cout << std::endl;
    
    // Cleanup
    MPI_Group_free(&origin_group);
}
```

---

## PSCW Pattern - Origin Process (rank 0)

The origin **accesses** the target's memory.

```cpp
if (rank == 0) {
    // Start access epoch
    MPI_Win_start(target_group,  // Group of targets to access
                  0,              // Assert flags (0 = none)
                  win);           // Window object
    
    // Prepare data to send
    int send_data[5] = {1, 2, 3, 4, 5};
    
    // Transfer data to target
    MPI_Put(send_data,           // Source buffer (local data)
            5,                    // Number of elements to send
            MPI_INT,              // Source datatype
            1,                    // Target rank
            0,                    // Offset in target window (start at index 0)
            5,                    // Number of elements at target
            MPI_INT,              // Target datatype
            win);                 // Window object
    
    // Close access epoch (signals target we're done)
    MPI_Win_complete(win);
    
    // Cleanup
    MPI_Group_free(&target_group);
}
```

---

## Cleanup

Free groups and window before finalizing MPI.

```cpp
// Both processes: free world group
MPI_Group_free(&world_group);

// Both processes: free window
MPI_Win_free(&win);
```

---

## Complete PSCW Template

```cpp
#include <iostream>
#include <mpi.h>

int main(int argc, char** argv) {
    MPI_Init(&argc, &argv);
    
    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    
    MPI_Win win;
    int data[5] = {0, 0, 0, 0, 0};
    
    {
        // Get world group
        MPI_Group world_group;
        MPI_Comm_group(MPI_COMM_WORLD, &world_group);
        
        if (rank == 1) {  // TARGET
            // Create origin group
            MPI_Group origin_group;
            int origin_ranks[1] = {0};
            MPI_Group_incl(world_group, 1, origin_ranks, &origin_group);
            
            // Create window
            MPI_Win_create(&data, sizeof(int) * 5, sizeof(int),
                          MPI_INFO_NULL, MPI_COMM_WORLD, &win);
            
            // Print before
            std::cout << "Target BEFORE: ";
            for (int i = 0; i < 5; i++) std::cout << data[i] << " ";
            std::cout << std::endl;
            
            // PSCW: Post → Wait
            MPI_Win_post(origin_group, 0, win);
            MPI_Win_wait(win);
            
            // Print after
            std::cout << "Target AFTER: ";
            for (int i = 0; i < 5; i++) std::cout << data[i] << " ";
            std::cout << std::endl;
            
            // Cleanup
            MPI_Group_free(&origin_group);
            
        } else {  // ORIGIN
            // Create target group
            MPI_Group target_group;
            int target_ranks[1] = {1};
            MPI_Group_incl(world_group, 1, target_ranks, &target_group);
            
            // Create window
            int dummy[1];
            MPI_Win_create(dummy, sizeof(int), sizeof(int),
                          MPI_INFO_NULL, MPI_COMM_WORLD, &win);
            
            // PSCW: Start → Put → Complete
            MPI_Win_start(target_group, 0, win);
            
            int send_data[5] = {1, 2, 3, 4, 5};
            MPI_Put(send_data, 5, MPI_INT, 1, 0, 5, MPI_INT, win);
            
            MPI_Win_complete(win);
            
            // Cleanup
            MPI_Group_free(&target_group);
        }
        
        // Both: cleanup
        MPI_Group_free(&world_group);
    }
    
    MPI_Win_free(&win);
    MPI_Finalize();
}
```

# 10/15/2025

Working on building out the benchmark files for KokkosComm.

## Parameter Quick Reference Card
```
fence()
  └─ assert (optional, default=0)

put(data, count, target, offset)
  ├─ data:   pointer to send
  ├─ count:  how many elements
  ├─ target: which rank to write to
  └─ offset: position in their window

get(data, count, source, offset)
  ├─ data:   pointer to receive into
  ├─ count:  how many elements
  ├─ source: which rank to read from
  └─ offset: position in their window
```

# 10/14/2025

Cleaned up files. Still working on tasks from last meeting and refining my understanding of Kokkos and MPI.

Built out the get, put, and fence functions into the MPI_Window class

Here is what is required:

```cpp
template <typename T>
void get(T* origin_addr,        // Where to store data I'm getting
         int count,              // How many elements
         int source_rank,        // Who to Get from
         MPI_Aint source_disp);  // Offset in their window
```

# 10/7/2025

Did a little bit of Kokkos practice.

Next I need to write a test case for get/put. I might not be able to 
get to this until tomorrow.

# 10/6/2025

Going to continue working on Kokkos problems. 

Checklist of everything I've done and will be working on:

- ~~Go through Onboarding (up to data visualization)~~
- ~~Go through Kokkos problems (going to complete all)~~
- Write test case for get/put


# 10/5/2025

Working on Kokkos problems

## Compiling:

g++ -std=c++23 kokkos1.cpp -o my_practice \
    -I$HOME/install/kokkos/include \
    -L$HOME/install/kokkos/lib -lkokkoscore \
    -Wl,-rpath,$HOME/install/kokkos/lib

# 10/4/2025

Reviewed MPI tutorial on Livermore website (I have gone through about half of it before)

## Studied MPI:

### Initialization & Finalization

```MPI_Init(&argc, &argv);```
- Starts MPI

```MPI_Finalize();```
- Ends MPI

### Basic Info:

```cpp
int rank, size;
MPI_Comm_rank(MPI_COMM_WORLD, &rank);
// Gets the process ID (0 to size - 1)
MPI_Comm_size(MPI_COMM_WORLD, &size);
// Get total number of processes
```

### Compilation:

```mpicxx -o program program.cpp```

- Compiles the program.

```mpirun -n 2 ./program```

- Runs the program with n number of processes.

### Blocking Point-To-Point:

```cpp
MPI_Send(&data, 			// Buffer to send
		count, 				// Number of elements
		MPI_DATATYPE, 		//MPI_INT, MPI_DOUBLE, etc
		dest_rank, 			// Target process
		tag, 				// Message tag (usually 0)
		MPI_COMM_WORLD); 	// Communicator
```
```cpp
MPI_Recv(&buffer, 			// Where to store received data
		count, 				// Max elements to receive
		MPI_DATATYPE, 		// MPI_INT, MPI_DOUBLE, etc
		source_rank, 		// Source process
		tag, 				// Message tag or MPI_Any_TAG
		MPI_COMM_WORLD, 	// Communicator
		MPI_STATUS_IGNORE); // Status
```
### Non-Blocking Point-To-Point:

```MPI_Request request; ```

```cpp
MPI_Isend(&data, 		// Buffer to send 
		count, 			// Number of elements
		MPI_DATATYPE, 	// Datatype like above
		dest_rank, 		// Target process
		tag, 			// Message tag (usually 0)
		MPI_COMM_WORLD, // Communicator
		&request
		);
```

```cpp
MPI_Irecv(&buffer, 		// Where to store received data
		count, 			// Max elements to receive
		MPI_DATATYPE, 	// Datatype like above
		source_rank, 	// Source process
		tag, 			// Message tag (usually 0)
		MPI_COMM_WORLD, // Communicator
		&request
		);
```

```cpp
MPI_Wait(&request, MPI_STATUS_IGNORE);
```

### Collective Operations:

```cpp
// Synchronization barrier
MPI_Barrier(MPI_COMM_WORLD);
```
- All processes wait here

```cpp
// Reduction - combine values from all processes
MPI_Reduce(&local_value, 	// Send buffer
		&global_result, 	// Receive buffer
		count, 				// Number of elements
		MPI_DATATYPE, 		// Data type
		MPI_SUM, 			// Operation
		root_rank 			// Process that gets the result
		MPI_COMM_WORLD
		);
```

### Common MPI_DATATYPEs:
```cpp
MPI_INT            // int
MPI_LONG           // long
MPI_LONG_LONG      // long long
MPI_FLOAT          // float
MPI_DOUBLE         // double
MPI_CHAR           // char
MPI_BYTE           // Generic byte
```

### Common MPI Constants:
```cpp
MPI_COMM_WORLD     // Default communicator (all processes)
MPI_ANY_SOURCE     // Receive from any process
MPI_ANY_TAG        // Accept any tag
MPI_STATUS_IGNORE  // Don't need status info
MPI_STATUSES_IGNORE // Plural for Waitall
MPI_INFO_NULL      // No special hints
```

### Common MPI_Reduce Operations:
```cpp
MPI_SUM // Add all values
MPI_MIN // Find minimum
MPI_MAX // Find maximum
MPI_PROD // Multiply all values
```


Built monte-carlo simulation serial baseline
Built MPI Monte-Carlo simulation with parallelization