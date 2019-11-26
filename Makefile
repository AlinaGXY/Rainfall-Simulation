all: rainfall_seq rainfall_pt

rainfall_seq: rainfall_seq.cpp landscape.hpp point.hpp
	g++ -std=c++11 -O3 -o rainfall_seq rainfall_seq.cpp

rainfall_pt: rainfall_pt.cpp landscape.hpp point.hpp
	g++ -std=c++11 -O3 -pthread -o rainfall_pt rainfall_pt.cpp

.PHONY: 
clean:
	rm *~ *.o
	