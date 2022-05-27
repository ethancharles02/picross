#include "Functions.h"

#include <windows.h>
#include <iostream>

// Returns true or false based on the percent (as a decimal). Accurate to 6 decimal places.
bool rand_chance(double percent) {
	// Skips any processing if the percent is equal to or above 100%
	if (percent >= 1) {
		return true;
	}

	// Counts the decimal places to decide the range for calculating the change, more decimals goes up in range since they need to be integers
	int num_decimals = 0;
	double num = percent;
	while (num_decimals <= 6) {
		num = num - int(num);
		if (num * 10 > 0) {
			num_decimals++;
			num *= 10;
		}
		else {
			break;
		}
	}

	int decimal_range = pow(10, num_decimals);

	bool result = rand() % decimal_range < percent* decimal_range;

	return result;
}