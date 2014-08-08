#include <mutex>
#include <queue>
#include <atomic>
#include <chrono>
#include <random>
#include <string>
#include <thread>
#include <vector>
#include <sstream>
#include <iostream>

#include "complex.h"
#include "settings.h"
#include "atom_wrapper.hpp"
#include "seed_generator.h"

#define DEVOUT std::cerr << __LINE__ << std::endl;

typedef unsigned int uint;
typedef unsigned long long uint64;

bool outside(Complex x)
{
	return x.im < LEFT || x.im > RIGHT || x.re < TOP || x.re > BOTTOM;
}

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

void flush_pic(std::vector<AtomWrapper<uint64>>& pic, std::string filename)
{
	FILE * output = fopen(filename.c_str(), "wb");
	fwrite(&pic[0], sizeof(uint64), SIDE * SIDE, output);
	fclose(output);
}

template <typename T>
std::string to_string(T val)
{
	std::ostringstream sstream;
	sstream << val;
	return sstream.str();
}

void generate(int num, std::vector<AtomWrapper<uint64>>& pic, SeedGenerator& random, std::mutex& rand_mutex, std::mutex& cerr_mutex, std::vector<AtomWrapper<uint>>& semaphores, const std::vector<uint64>& targets)
{
	std::vector<Complex> sequence;
	std::queue<Complex> rand_queue;
	size_t curr_target_index = 0;

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
		
		if (seed_it == targets[curr_target_index] / THREADS_NUM)
		{
			semaphores[curr_target_index]++;
			if (num == 0)
			{
				while (semaphores[curr_target_index].load() != THREADS_NUM)
				{
					std::this_thread::yield();
				}
				flush_pic(pic, "pic_" + to_string(targets[curr_target_index]) + ".bin");
				semaphores[curr_target_index].store(0);
			}
			else
			{
				while (semaphores[curr_target_index].load() != 0)
				{
					std::this_thread::yield();
				}
			}
			curr_target_index++;
		}
		
		if (seed_it % (SEED_ITERATIONS / 119 / THREADS_NUM) == num * SEED_ITERATIONS / 119 / THREADS_NUM / THREADS_NUM)
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
	
	SeedGenerator random; //std::vector<bool>(CELLS_PER_SIDE * CELLS_PER_SIDE, true));
	print(start, "Random initialized");
	
	std::mutex rand_mutex;
	std::mutex cerr_mutex;
	
	std::vector<AtomWrapper<uint64>> pic(SIDE * SIDE, std::atomic<uint64>(0));
	std::vector<uint64> targets;
	{
		uint64 curr_target = MIN_SEED_ITERATIONS;
		while (curr_target < SEED_ITERATIONS)
		{
			targets.push_back(curr_target);
			targets.push_back(curr_target * 2.5);
			targets.push_back(curr_target * 5);
			curr_target *= 10;
		}
	}
	for (uint64 i : targets)
	{
		std::cerr << i << std::endl;
	}
	std::vector<AtomWrapper<uint>> semaphores(targets.size(), std::atomic<uint>(0));
	std::vector<std::thread> threads(THREADS_NUM);
	print(start, "Memory allocated");
	
	for (int i = 0; i < THREADS_NUM; i++)
	{
		threads[i] = std::thread(generate, i, std::ref(pic), std::ref(random), std::ref(rand_mutex), std::ref(cerr_mutex), std::ref(semaphores), std::ref(targets));
	}

	for (int i = 0; i < THREADS_NUM; i++)
	{
		threads[i].join();
	}
	std::cerr << std::endl;
	print(start, "Threads ended");

	flush_pic(pic, "pic.bin");
	print(start, "Data written");
}