#include <iostream>
#include <Kokkos_Core.hpp>

int main(int argc, char* argv[]) {
    Kokkos::initialize(argc, argv);
    {
        Kokkos::view<double*> a("A",1);

        std::cout << a.label() << std::endl;
    }
    Kokkos::finalize();
    return 0;
}