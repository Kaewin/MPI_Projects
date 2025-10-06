# 10/5/2025

Working on Kokkos problems

## Compiling:

g++ -std=c++20 kokkos1.cpp -o my_practice \
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