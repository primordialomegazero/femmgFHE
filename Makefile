# Fibonacci FHE — Makefile
# Professional build system

CXX := g++
CXXFLAGS := -std=c++17 -O3 -march=native -I/usr/include -I.
LDFLAGS := -lntl -lgmp -lm

# Directories
SRC_DIR := src
TEST_DIR := tests
BUILD_DIR := build
RESULTS_DIR := results

# Create build directory if not exists
$(shell mkdir -p $(BUILD_DIR))

# Source files
FHE_HEADERS := $(SRC_DIR)/fhe/golden_fibonacci_fhe_v5.h
IO_HEADERS := $(SRC_DIR)/io/golden_fibonacci_io_v2.h
QUANTUM_HEADERS := $(SRC_DIR)/quantum/golden_fibonacci_quantum_v2.h

# Test targets
TESTS := \
    test_complete_pipeline \
    test_full_adder_complete \
    test_4bit_adder \
    test_2048_full_quantum \
    test_257_stress \
    test_1024_stress

# Build all tests
.PHONY: all
all: $(TESTS)

# Individual test builds
.PHONY: test_complete_pipeline
test_complete_pipeline:
	$(CXX) $(CXXFLAGS) $(TEST_DIR)/test_complete_pipeline.cpp -o $(BUILD_DIR)/$@ $(LDFLAGS)

.PHONY: test_full_adder_complete
test_full_adder_complete:
	$(CXX) $(CXXFLAGS) $(TEST_DIR)/test_full_adder_complete.cpp -o $(BUILD_DIR)/$@ $(LDFLAGS)

.PHONY: test_4bit_adder
test_4bit_adder:
	$(CXX) $(CXXFLAGS) $(TEST_DIR)/test_4bit_ripple_adder.cpp -o $(BUILD_DIR)/$@ $(LDFLAGS)

.PHONY: test_2048_full_quantum
test_2048_full_quantum:
	$(CXX) $(CXXFLAGS) $(TEST_DIR)/test_2048bit_full_adder_quantum.cpp -o $(BUILD_DIR)/$@ $(LDFLAGS)

.PHONY: test_257_stress
test_257_stress:
	$(CXX) $(CXXFLAGS) $(TEST_DIR)/test_257bit_100k_stress.cpp -o $(BUILD_DIR)/$@ $(LDFLAGS)

.PHONY: test_1024_stress
test_1024_stress:
	$(CXX) $(CXXFLAGS) $(TEST_DIR)/test_1024bit_100k_stress.cpp -o $(BUILD_DIR)/$@ $(LDFLAGS)

# Run tests
.PHONY: run-pipeline
run-pipeline: test_complete_pipeline
	./$(BUILD_DIR)/test_complete_pipeline

.PHONY: run-full-adder
run-full-adder: test_full_adder_complete
	./$(BUILD_DIR)/test_full_adder_complete

.PHONY: run-4bit-adder
run-4bit-adder: test_4bit_adder
	./$(BUILD_DIR)/test_4bit_adder

.PHONY: run-2048-quantum
run-2048-quantum: test_2048_full_quantum
	./$(BUILD_DIR)/test_2048_full_quantum

# Clean build artifacts
.PHONY: clean
clean:
	rm -rf $(BUILD_DIR)/*
	rm -f test_* *.o

# Clean everything including results
.PHONY: clean-all
clean-all: clean
	rm -f $(RESULTS_DIR)/*.txt

# Help
.PHONY: help
help:
	@echo "Fibonacci FHE Build System"
	@echo ""
	@echo "Targets:"
	@echo "  all              - Build all tests"
	@echo "  test_*           - Build individual test"
	@echo "  run-pipeline     - Run complete pipeline test"
	@echo "  run-full-adder   - Run full adder test"
	@echo "  run-4bit-adder   - Run 4-bit ripple adder test"
	@echo "  run-2048-quantum - Run 2048-bit quantum test"
	@echo "  clean            - Remove build artifacts"
	@echo "  clean-all        - Remove build artifacts and results"
	@echo "  help             - Show this help"
