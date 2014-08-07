#pragma once

#include <unordered_set>
#include <random>
#include <vector>

#include "complex.h"

class SeedGenerator
{
public:
	SeedGenerator();
	
	SeedGenerator(std::vector<bool> interesting_cells);

	Complex generate();

private:
	bool is_interesting(Complex c);
	
	bool is_interesting(int cell_number);

	std::mt19937_64 generator;
	std::uniform_real_distribution<double> distribution;
	std::unordered_set<size_t> interesting_cells_indices;
	std::unordered_set<size_t>::const_iterator next_cell;
};