all: b complex
	g++ b.o complex.o -o b
	
b:
	g++ -std=c++11 -O3 -c b.cpp
	
complex:
	g++ -std=c++11 -O3 -c complex.cpp