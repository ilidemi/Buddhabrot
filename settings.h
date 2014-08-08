#pragma once

// Image properties
#define SIDE 42519
#define ANTIALIASING 3
#define LEFT (-5.0 / 3)
#define RIGHT (5.0 / 3)
#define TOP (-13.0 / 6)
#define BOTTOM (7.0 / 6)

// Sampling grid properties
#define CELLS_PER_SIDE 100
#define CELL_SEED_ITERATIONS 10000
#define CELL_ITERATIONS 1000
#define TOTAL_CELLS (CELLS_PER_SIDE * CELLS_PER_SIDE)

// Multithreading properties
#define THREADS_NUM 8
#define RANDOM_SIZE 1000000

// Generation properties
#define MIN_SEED_ITERATIONS 200000
#define SEED_ITERATIONS (1ULL * 2 * 1000 * 1000 * 1000 * 1000)
#define MIN_ITERATIONS 0
#define MAX_ITERATIONS 100000