.PHONY: all test benchmark clean

CXX = g++
CXXFLAGS = -std=c++17 -O3 -march=native -pthread
INCLUDES = -I src/core -I src/chaos -I src/security -I src/kem -I src/storage -I src/math
LIBS = -lm -lssl -lcrypto

all: test benchmark

test:
	$(CXX) $(CXXFLAGS) $(INCLUDES) -o test_suite tests/test_suite.cpp $(LIBS)
	./test_suite

benchmark:
	$(CXX) -O0 $(INCLUDES) -o test_ctu5_benchmark tests/test_ctu5_benchmark.cpp $(LIBS)
	./test_ctu5_benchmark

integration:
	$(CXX) $(CXXFLAGS) $(INCLUDES) -o test_ctu5_integration tests/test_ctu5_integration.cpp $(LIBS)
	./test_ctu5_integration

clean:
	rm -f test_suite test_ctu5_benchmark test_ctu5_integration
