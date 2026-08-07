CXX = g++
CXXFLAGS = -std=c++17 -O3 -Wall
INCLUDES = -I../../src -I../..

all: test_ecdlp_sat_encoder

test_ecdlp_sat_encoder: test_ecdlp_sat_encoder.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDES) -o test_ecdlp_sat_encoder test_ecdlp_sat_encoder.cpp

run: test_ecdlp_sat_encoder
	./test_ecdlp_sat_encoder

clean:
	rm -f test_ecdlp_sat_encoder *.cnf
