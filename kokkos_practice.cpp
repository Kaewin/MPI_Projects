#include <iostream>
#include <Kokkos_Core.hpp>

int main(int argc, char** argv) {
	Kokkos::initialize(argc, argv);
	{
		int n = 100;
		Kokkos::View<int*> array("Array1", n);

		// Fill array wth integers up to 100:
		Kokkos::parallel_for(n, KOKKOS_LAMBDA(int i) {
			array(i) = i + 1;
		});

		int sum = 0;
		Kokkos::parallel_reduce(n, KOKKOS_LAMBDA(int i, int& lsum) {
			lsum += array(i);
		}, sum);

		std::cout << "Sum of 1 to 100 " << sum << std::endl;

		// Find maximum
		int maximum = 0;
		Kokkos::parallel_reduce(n, KOKKOS_LAMBDA(int i, int& lmax){
			if (array(i) > maximum) {
				lmax = array(i);
			}
		}, Kokkos::Max<int>(maximum));

		std::cout << "Maximum Value: " << maximum << std::endl;

		// Copy to host:
		auto view_host = Kokkos::create_mirror_view_and_copy(
			Kokkos::HostSpace(), array);
		
		// Print on CPU
		for (int i = 0; i < n; i++) {
			std::cout << view_host(i) << "\t";
		}
		std::cout << std::endl;
	}
	Kokkos::finalize();
	return 0;
}