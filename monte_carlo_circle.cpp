#include <iostream>
#include <random>

int main() {
	int total_points = 1000000;
	int points_in_circle = 0;

	// Get randomness from hardware
	std::random_device rd;
	// Random number generation algorithm: Mersenne Twister, period 2^19937-1
	std::mt19937 gen(rd());
	// Generate a probability distribution: a uniform real distribution between 0 and 1
	std::uniform_real_distribution<> dis(0.0, 1.0);

	// Throw random darts
	for(int i = 0; i < total_points; i++) {
		double x = dis(gen);
		double y = dis(gen);

		// Check if inside circle
		// Using quarter of a unit circle
		if(x*x + y*y <= 1.0) {
			points_in_circle++;
		}
	}

	// Multiply by 4 to isolate PI, given first quadrant is pi/4;
	double pi_estimate = 4.0 * points_in_circle / total_points;
	std::cout << "PI Estimate: " << pi_estimate << std::endl;

	return 0;
}

