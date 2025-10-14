# MPI_Projects

# MPI Compile

Compile: mpicc [program_name].cpp

Run: mpiexec -n [number_of_cores] [executable_name]

## Kokkos Compile:

g++ -std=c++23 kokkos1.cpp -o my_practice
-I$HOME/install/kokkos/include
-L$HOME/install/kokkos/lib -lkokkoscore
-Wl,-rpath,$HOME/install/kokkos/lib
