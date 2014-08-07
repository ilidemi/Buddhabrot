#include "seed_generator.h"

#include <iostream>

#include "settings.h"

extern bool outside(Complex x);

SeedGenerator::SeedGenerator()
{
	for (int i = 0; i < TOTAL_CELLS; i++)
	{
		if (is_interesting(i))
		{
			interesting_cells_indices.insert(i);
		}
	}
	next_cell = interesting_cells_indices.end();
	
	for (int i = 0; i < TOTAL_CELLS; i++)
	{
		if (interesting_cells_indices.count(i) > 0)
		{
			std::cerr << 'X';
		}
		else
		{
			std::cerr << ' ';
		}
		if (i % CELLS_PER_SIDE == CELLS_PER_SIDE - 1)
		{
			std::cerr << std::endl;
		}
	}
}

SeedGenerator::SeedGenerator(std::vector<bool> interesting_cells)
{
	for (int i = 0; i < TOTAL_CELLS; i++)
	{
		if (interesting_cells[i])
		{
			interesting_cells_indices.insert(i);
		}
	}
	next_cell = interesting_cells_indices.end();
}
	
Complex SeedGenerator::generate()
{
	if (next_cell == interesting_cells_indices.end())
	{
		next_cell = interesting_cells_indices.begin();
	}
	
	size_t cell_number = *next_cell;
	
	const double cell_height = (BOTTOM - TOP) / CELLS_PER_SIDE;
	const double cell_width = (RIGHT - LEFT) / CELLS_PER_SIDE;
	
	int cell_x = cell_number % CELLS_PER_SIDE;
	int cell_y = cell_number / CELLS_PER_SIDE;
	double cell_top = TOP + cell_y * cell_height;
	double cell_left = LEFT + cell_x * cell_width;
	
	Complex result;
	result.re = cell_top + distribution(generator) * cell_height;
	result.im = cell_left + distribution(generator) * cell_width;
	
	next_cell++;
	return result;
}

bool SeedGenerator::is_interesting(Complex c)
{
	int cell_x = (c.re - LEFT) / (RIGHT - LEFT) * CELLS_PER_SIDE;
	int cell_y = (c.im - TOP) / (BOTTOM - TOP) * CELLS_PER_SIDE;
	return interesting_cells_indices.count(cell_y * CELLS_PER_SIDE + cell_x) > 0;
}

bool SeedGenerator::is_interesting(int cell_number)
{
	const double cell_height = (BOTTOM - TOP) / CELLS_PER_SIDE;
	const double cell_width = (RIGHT - LEFT) / CELLS_PER_SIDE;
	
	int cell_x = cell_number % CELLS_PER_SIDE;
	int cell_y = cell_number / CELLS_PER_SIDE;
	double cell_top = TOP + cell_y * cell_height;
	double cell_left = LEFT + cell_x * cell_width;
	
	bool contains_interior = false;
	bool contains_exterior = false;
	
	for (int seed_it = 0; seed_it < CELL_SEED_ITERATIONS && !(contains_interior && contains_exterior); seed_it++)
	{
		Complex c, x;
		c.re = cell_top + cell_height * distribution(generator);
		c.im = cell_left + cell_width * distribution(generator);
		x = c;
		
		for (int it = 0; it < CELL_ITERATIONS; it++)
		{
			x = x * x + c;
			if (outside(x))
			{
				contains_exterior = true;
				break;
			}
		}
		if (!outside(x))
		{
			contains_interior = true;
		}
	}
	
	return contains_interior && contains_exterior;
}