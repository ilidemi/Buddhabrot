all: b.o complex.o seed_generator.o
	g++ b.o complex.o seed_generator.o -o b
	
b.o: b.cpp
	g++ -std=c++11 -O3 -c b.cpp
	
complex.o: complex.cpp
	g++ -std=c++11 -O3 -c complex.cpp
	
seed_generator.o: seed_generator.cpp
	g++ -std=c++11 -O3 -c seed_generator.cpp
	
clean:
	rm -f ./*.o ./b