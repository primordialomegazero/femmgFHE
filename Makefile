# ═══════════════════════════════════════════════════════════════════════════════
# SPIRAL FRACTAL iO — MAKEFILE v30.0
# ═══════════════════════════════════════════════════════════════════════════════

.PHONY: all build test unit-test integration-test quick-test docker clean

OPENFHE_LIB = ./openfhe-development/build/lib
OPENFHE_INC = -I./openfhe-development/src/pke/include \
              -I./openfhe-development/src/core/include \
              -I./openfhe-development/src/binfhe/include \
              -I./openfhe-development/build/src/core \
              -I. -I./src
OPENFHE_LINK = -L$(OPENFHE_LIB) -lOPENFHEpke -lOPENFHEcore -lOPENFHEbinfhe \
               -Wl,-rpath,$(OPENFHE_LIB) -lsodium -lsqlite3 -lstdc++ -lpthread -lm
CXX_FLAGS = -std=c++17 -O3 -march=native -mtune=native

# ═══════════════════════════════════════════════════════════════════════════════
# ALL — Builds OpenFHE + all core binaries
# ═══════════════════════════════════════════════════════════════════════════════
all: openfhe core-binaries unit-binaries kem

openfhe:
	@if [ ! -f $(OPENFHE_LIB)/libOPENFHEpke.so ]; then \
		echo "Building OpenFHE (one-time)..."; \
		cd openfhe-development && mkdir -p build && cd build && \
		cmake .. -DWITH_OPENMP=OFF -DCMAKE_BUILD_TYPE=Release && make -j$$(nproc); \
		echo "OpenFHE build complete."; \
	fi

# ═══════════════════════════════════════════════════════════════════════════════
# CORE BINARIES
# ═══════════════════════════════════════════════════════════════════════════════
core-binaries: bin/test_io_batched bin/test_io_16k_spiral bin/test_unified_all bin/test_full_integration

bin/test_io_batched: tests/breakthrough/test_io_batched.cpp
	g++ $(CXX_FLAGS) $(OPENFHE_INC) -o $@ $< $(OPENFHE_LINK)

bin/test_io_16k_spiral: tests/breakthrough/test_io_16k_spiral.cpp
	g++ $(CXX_FLAGS) $(OPENFHE_INC) -o $@ $< $(OPENFHE_LINK)

bin/test_unified_all: unified-phi-stack/test_unified.cpp unified-phi-stack/phi_stack.h
	g++ $(CXX_FLAGS) -I./unified-phi-stack -o $@ $< -lm

bin/test_full_integration: tests/breakthrough/test_full_integration.cpp
	g++ $(CXX_FLAGS) $(OPENFHE_INC) -o $@ $< $(OPENFHE_LINK)

# ═══════════════════════════════════════════════════════════════════════════════
# UNIT BINARIES
# ═══════════════════════════════════════════════════════════════════════════════
unit-binaries: bin/test_hierarchical_seed bin/test_fractal_chaos bin/test_spiral_bootstrap

bin/test_hierarchical_seed: tests/unit/test_hierarchical_seed.cpp
	g++ $(CXX_FLAGS) -I./src -o $@ $< -lm

bin/test_fractal_chaos: tests/unit/test_fractal_chaos.cpp
	g++ $(CXX_FLAGS) -I./src -o $@ $< -lm

bin/test_spiral_bootstrap: tests/unit/test_spiral_bootstrap.cpp
	g++ $(CXX_FLAGS) $(OPENFHE_INC) -o $@ $< $(OPENFHE_LINK)

# ═══════════════════════════════════════════════════════════════════════════════
# KEM
# ═══════════════════════════════════════════════════════════════════════════════
kem: bin/phi_kem_level5

bin/phi_kem_level5: src/kem/phi_kem_level5.c
	gcc -std=c99 -O3 -o $@ $< -lcrypto -lm

# ═══════════════════════════════════════════════════════════════════════════════
# TEST SUITES
# ═══════════════════════════════════════════════════════════════════════════════
test: quick-test integration-test

quick-test:
	@echo "╔══════════════════════════════════════════════════════════════╗"
	@echo "║  QUICK TEST SUITE                                           ║"
	@echo "╚══════════════════════════════════════════════════════════════╝"
	@echo ""
	@echo "=== Unified Phi Stack ==="
	./bin/test_unified_all
	@echo ""
	@echo "=== Hierarchical Seed ==="
	./bin/test_hierarchical_seed
	@echo ""
	@echo "=== Fractal Chaos ==="
	./bin/test_fractal_chaos
	@echo ""
	@echo "=== KEM Level 5 ==="
	./bin/phi_kem_level5
	@echo ""
	@echo "╔══════════════════════════════════════════════════════════════╗"
	@echo "║  QUICK TEST SUITE COMPLETE                                  ║"
	@echo "╚══════════════════════════════════════════════════════════════╝"

unit-test: unit-binaries
	@echo "Running all unit tests..."
	./bin/test_hierarchical_seed
	./bin/test_fractal_chaos
	@echo "Unit tests complete."

integration-test: bin/test_full_integration
	@echo "Running integration test..."
	LD_LIBRARY_PATH=$(OPENFHE_LIB):$$LD_LIBRARY_PATH ./bin/test_full_integration

# ═══════════════════════════════════════════════════════════════════════════════
# iO VALIDATION
# ═══════════════════════════════════════════════════════════════════════════════
run-dev: bin/test_io_batched
	LD_LIBRARY_PATH=$(OPENFHE_LIB):$$LD_LIBRARY_PATH ./bin/test_io_batched 10 3

run-test: bin/test_io_batched
	LD_LIBRARY_PATH=$(OPENFHE_LIB):$$LD_LIBRARY_PATH ./bin/test_io_batched 50 5

run-16k: bin/test_io_16k_spiral
	LD_LIBRARY_PATH=$(OPENFHE_LIB):$$LD_LIBRARY_PATH ./bin/test_io_16k_spiral 10 5

# ═══════════════════════════════════════════════════════════════════════════════
# DOCKER
# ═══════════════════════════════════════════════════════════════════════════════
docker:
	docker build -t spiralfractalio .

docker-run:
	docker run -p 8443:8443 -v $$(pwd)/data:/app/data spiralfractalio

# ═══════════════════════════════════════════════════════════════════════════════
# CLEAN
# ═══════════════════════════════════════════════════════════════════════════════
clean:
	rm -f bin/test_io_batched bin/test_io_16k_spiral bin/test_unified_all
	rm -f bin/test_hierarchical_seed bin/test_fractal_chaos bin/test_spiral_bootstrap
	rm -f bin/test_full_integration bin/phi_kem_level5
	@echo "Binaries cleaned."
