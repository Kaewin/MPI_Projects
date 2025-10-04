# 10/4/2025

Reviewed MPI tutorial on Livermore website (I have gone through about half of it before)

## Studied MPI:

### Initialization & Finalization

MPI_Init(&argc, &argv);
- Starts MPI

MPI_Finalize();
- Ends MPI

### Basic Info:

```cpp
int rank, size;
MPI_Comm_rank(MPI_COMM_WORLD, &rank);
// Gets the process ID (0 to size - 1)
MPI_Com_size(MPI_COMM_WORLD, &size);
// Get total number of processes
```

### Compilation:

```mpicxx -o program program.cpp```

- Compiles the program.

```mpirun -n 2 ./program```

- Runs the program with n number of processes.

### Blocking Point-To-Point:

```cpp
MPI_Send()


Built monte-carlo simulation serial baseline
Built MPI Monte-Carlo simulation with parallelization

