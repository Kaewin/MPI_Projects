/*
Link and run program with Kokkos where you initialize a View and print out its name with the .label()
method.
*/
#include <iostream>
#include <Kokkos_Core.hpp>

int main(int argc, char** argv) {
    	Kokkos::initialize(argc, argv);
    	{
		// do something in scope
    	}
    	Kokkos::finalize();
    	return 0;
}