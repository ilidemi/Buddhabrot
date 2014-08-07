all: b complex seed_generator
	g++ b.o complex.o seed_generator.o -o b
	
b:
	g++ -std=c++11 -O3 -c b.cpp
	
complex:
	g++ -std=c++11 -O3 -c complex.cpp
	
seed_generator:
	g++ -std=c++11 -O3 -c seed_generator.cpp
	
clean:
	rm -f ./*.o ./b