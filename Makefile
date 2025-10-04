# Makefile

all:	MPI_Test

MPI_Test: mpi_monte_carlo_circle_timed.cpp, mpi_monte_carlo_circle_timed_non_blocking.cpp
	mpicxx -o monte_blocking mpi_monte_carlo_circle_timed.cpp
	mpicxx -o monte_non_blocking mpi_monte_carlo_circle_timed_non_blocking.cpp

clean: monte_blocking, monte_non_blocking
	rm monte_blocking monte_non_blocking	