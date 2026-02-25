// What in the world is this? Look it up.
#include <iostream>
#include <mpi.h>

using namespace std;

void mainloop(int P, N);

int main(int argc, char** argv) {
	MPI_Init(&argc, &argv);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	MPI_Comm_size(MPI_COMM_WORLD, &size);

	int P, int N;

	cout << "Enter partitions; "; 
	cin >> P;
	cout << "Enter entries: ";
	cin >> N;

	int L = N/P; // Nominal amount of entries per partition
	int R = N%P; // Any extras that don't fit

	// For an index I, what partition does it go into 
	// And at what offset?

	int p, i;
	int I = 0; // Some entry for 0...N-1

	cout << "Enter the index to partition: ";
	cin >> I;

	// for b!=0, (a/b)*b = a - a mod b
	if(R == 0) 
	{
		p = I/L;
		i = I-p*L;
	}
	else // Non-divisibility
	{
		if(I < (L+1)*R)
		{
			p = I/(L+1);
			i = I - (L+1)*p;
		}
		else 
		{
			p = R + (I - (L + 1)*R)/L;
			i = I - R*(L+1) - (p-R)*L;
		}
	}

	MPI_Finalize();
	return 0;
}