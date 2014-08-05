#include <ctime>
#include <mutex>
#include <queue>
#include <atomic>
#include <chrono>
#include <cstdio>
#include <random>
#include <thread>
#include <vector>
#include <cstdlib>
#include <cstring>
#include <iomanip>
#include <iostream>

#define DEVOUT std::cerr << __LINE__ << std::endl;

// Image properties
#define SIDE 8000
#define ANTIALIASING 4
#define LEFT (-5.0 / 3)
#define RIGHT (5.0 / 3)
#define TOP (-13.0 / 6)
#define BOTTOM (7.0 / 6)

// Sampling grid properties
#define CELLS_PER_SIDE 100
#define CELL_SEED_ITERATIONS 1000
#define CELL_ITERATIONS 1000
#define TOTAL_CELLS (CELLS_PER_SIDE * CELLS_PER_SIDE)

// Multithreading properties
#define THREADS_NUM 4	
#define RANDOM_SIZE 1000000

// Generation properties
#define SEED_ITERATIONS (1ULL * 100 * 1000 * 1000)
#define MIN_ITERATIONS 0
#define MAX_ITERATIONS 100000

typedef unsigned int uint;
typedef unsigned long long uint64;

struct Complex
{
	double re;
	double im;

	Complex operator+(const Complex other) const
	{
		return{ re + other.re, im + other.im };
	}
	
	Complex operator-(const Complex other) const
	{
		return { re - other.re, im - other.im };
	}

	Complex operator*(const Complex other) const
	{
		return{ re * other.re - im * other.im, re * other.im + im * other.re };
	}

	bool operator==(const Complex other) const
	{
		return re == other.re && im == other.im;
	}
	
	double abs() const
	{
		return re*re + im*im;
	}
};

bool outside(Complex x)
{
	return x.im < LEFT || x.im > RIGHT || x.re < TOP || x.re > BOTTOM;
}

class SeedGenerator
{
public:
	SeedGenerator()
	{
		interesting_cells = std::vector<bool>(TOTAL_CELLS, false);
		for (int i = 0; i < TOTAL_CELLS; i++)
		{
			interesting_cells[i] = is_interesting(i);
		}
	}

	Complex generate()
	{
		Complex result;
		do
		{
			result.re = TOP + distribution(generator) * (BOTTOM - TOP);
			result.im = LEFT + distribution(generator) * (RIGHT - LEFT);
		} while (!is_interesting(result));
		return result;
	}

private:
	bool is_interesting(Complex c)
	{
		int cell_x = (c.re - LEFT) / (RIGHT - LEFT) * CELLS_PER_SIDE;
		int cell_y = (c.im - TOP) / (BOTTOM - TOP) * CELLS_PER_SIDE;
		return interesting_cells[cell_y * CELLS_PER_SIDE + cell_x];
	}
	
	bool is_interesting(int cell_number)
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
			c.re = cell_left + cell_width * distribution(generator);
			c.im = cell_top + cell_height * distribution(generator);
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
			contains_interior = true;
		}
		
		return contains_interior && contains_exterior;
	}

	std::mt19937_64 generator;
	std::uniform_real_distribution<double> distribution;
	std::vector<bool> interesting_cells;
};

template <typename T>
struct AtomWrapper
{
	std::atomic<T> _a;

	AtomWrapper()
		:_a()
	{}

	AtomWrapper(const std::atomic<T> &a)
		:_a(a.load())
	{}

	AtomWrapper(const AtomWrapper &other)
		:_a(other._a.load())
	{}

	AtomWrapper & operator=(const AtomWrapper &other)
	{
		_a.store(other._a.load());
	}
	
	T operator++()
	{
		return ++_a;
	}
	
	T operator++(int)
	{
		return _a++;
	}
	
	T operator+=(T val)
	{
		return _a.fetch_add(val);
	}
};

void inc(std::vector<AtomWrapper<uint64>>& pic, Complex x, uint amount)
{
	double pix_side = (RIGHT - LEFT) / SIDE;
	Complex pix { (x.re - TOP) / pix_side, (x.im - LEFT) / pix_side };
	Complex cand;
	
	for (int delta_x = -ANTIALIASING / 2; delta_x <= ANTIALIASING / 2; delta_x++)
	{
		int cand_pix_x = (int)pix.im + delta_x;
		if (cand_pix_x < 0 || cand_pix_x >= SIDE)
		{
			continue;
		}
		cand.im = cand_pix_x + 0.5;
		for (int delta_y = -ANTIALIASING / 2; delta_y <= ANTIALIASING / 2; delta_y++)
		{
			int cand_pix_y = (int)pix.re + delta_y;
			if (cand_pix_y < 0 || cand_pix_y >= SIDE)
			{
				continue;
			}
			cand.re = cand_pix_y + 0.5;
			if ((cand - pix).abs() <= ANTIALIASING / 2)
			{
				pic[cand_pix_y * SIDE + cand_pix_x] += amount;
			}
		}
	}
}

bool is_power_of_two(uint x)
{
	return ((x != 0) && !(x & (x - 1)));
}

void fill_queue(SeedGenerator& random, std::mutex& rand_mutex, std::queue<Complex>& rand_queue)
{
	rand_mutex.lock();
	for (int i = 0; i < RANDOM_SIZE; i++)
	{
		rand_queue.push(random.generate());
	}
	rand_mutex.unlock();
}

void generate(int num, std::vector<AtomWrapper<uint64>>& pic, SeedGenerator& random, std::mutex& rand_mutex, std::mutex& cerr_mutex)
{
	std::vector<Complex> sequence;
	std::queue<Complex> rand_queue;

	for (uint64 seed_it = 0; seed_it < SEED_ITERATIONS / THREADS_NUM; seed_it++)
	{
		Complex c, x, old_x;
		if (rand_queue.empty())
		{
			fill_queue(random, rand_mutex, rand_queue);
		}
		old_x = x = c = rand_queue.front();
		rand_queue.pop();

		sequence.clear();
		sequence.reserve(MAX_ITERATIONS);

		bool repetitive = false;
		for (uint it = 0; it < MAX_ITERATIONS; it++)
		{
			x = x * x + c;

			if (outside(x))
			{
				break;
			}

			if (x == old_x)
			{
				repetitive = true;
				break;
			}

			sequence.push_back(x);

			if (is_power_of_two(it))
			{
				old_x = x;
			}
		}
		if (!repetitive && sequence.size() > MIN_ITERATIONS && sequence.size() != MAX_ITERATIONS)
		{
			for (size_t i = 0; i < sequence.size(); i++)
			{
				inc(pic, sequence[i], sequence.size());
			}
		}
		if (seed_it % (SEED_ITERATIONS / 119 / THREADS_NUM) == 0)
		{
			cerr_mutex.lock();
			std::cerr << '.';
			cerr_mutex.unlock();
		}
	}
}

void print(std::chrono::steady_clock::time_point& start, std::string message)
{
	std::cerr << std::chrono::duration_cast<std::chrono::duration<double>>(std::chrono::steady_clock::now() - start).count() << '\t' << message << std::endl;
}

int main()
{
	std::chrono::steady_clock::time_point start = std::chrono::steady_clock::now();
	print(start, "Start");
	
	SeedGenerator random;
	print(start, "Random initialized");
	
	std::mutex rand_mutex;
	std::mutex cerr_mutex;
	
	std::vector<AtomWrapper<uint64>> pic(SIDE * SIDE, std::atomic<uint64>(0));
	std::vector<std::thread> threads(THREADS_NUM);
	print(start, "Memory allocated");
	
	for (int i = 0; i < THREADS_NUM; i++)
	{
		threads[i] = std::thread(generate, i, std::ref(pic), std::ref(random), std::ref(rand_mutex), std::ref(cerr_mutex));
	}

	for (int i = 0; i < THREADS_NUM; i++)
	{
		threads[i].join();
	}
	std::cerr << std::endl;
	print(start, "Threads ended");

	FILE * output = fopen("pic.bin", "wb");
	fwrite(&pic[0], sizeof(uint64), SIDE * SIDE, output);
	fclose(output);
	print(start, "Data written");
}