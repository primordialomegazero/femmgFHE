# ═══════════════════════════════════════════════════════════════════════════════
# SPIRAL FRACTAL iO — MAKEFILE
# ═══════════════════════════════════════════════════════════════════════════════

.PHONY: all build test unit-test integration-test docker clean

# ── Build ──────────────────────────────────────────────────────────────────────
all: build

build:
	@echo "Building OpenFHE (one-time)..."
	cd openfhe-development && mkdir -p build && cd build && \
	cmake .. -DWITH_OPENMP=OFF -DCMAKE_BUILD_TYPE=Release && make -j$$(nproc)
	@echo "Building femmgFHE..."
	g++ -std=c++17 -O3 -march=native -mtune=native \
		-I./openfhe-development/src/pke/include \
		-I./openfhe-development/src/core/include \
		-I./openfhe-development/src/binfhe/include \
		-I./openfhe-development/build/src/core \
		-I. \
		-o bin/test_io_batched \
		tests/breakthrough/test_io_batched.cpp \
		-L./openfhe-development/build/lib \
		-lOPENFHEpke -lOPENFHEcore -lOPENFHEbinfhe \
		-Wl,-rpath,./openfhe-development/build/lib \
		-lstdc++ -lpthread -lm
	@echo "Build complete."

# ── Test ───────────────────────────────────────────────────────────────────────
test: unit-test integration-test

unit-test:
	@echo "Running unit tests..."
	g++ -std=c++17 -O0 -I. -o bin/test_safe_math tests/unit/test_safe_math.cpp -lm && ./bin/test_safe_math
	g++ -std=c++17 -O0 -I. -o bin/test_golden_fibonacci tests/unit/test_golden_fibonacci.cpp -lm && ./bin/test_golden_fibonacci
	g++ -std=c++17 -O0 -I. -o bin/test_fractal_chaos tests/unit/test_fractal_chaos.cpp -lm && ./bin/test_fractal_chaos
	g++ -std=c++17 -O0 -I. -o bin/test_system_config tests/unit/test_system_config.cpp -lm && ./bin/test_system_config
	@echo "Unit tests done."

integration-test:
	@echo "Running integration test..."
	LD_LIBRARY_PATH=./openfhe-development/build/lib:$$LD_LIBRARY_PATH ./bin/test_full_integration

# ── Run ────────────────────────────────────────────────────────────────────────
run-dev:
	LD_LIBRARY_PATH=./openfhe-development/build/lib:$$LD_LIBRARY_PATH ./bin/test_io_batched 10 3

run-test:
	LD_LIBRARY_PATH=./openfhe-development/build/lib:$$LD_LIBRARY_PATH ./bin/test_io_batched 50 5

run-batch:
	LD_LIBRARY_PATH=./openfhe-development/build/lib:$$LD_LIBRARY_PATH ./bin/test_batch_rich

# ── Docker ─────────────────────────────────────────────────────────────────────
docker:
	docker build -t spiralfractalio .

docker-run:
	docker run -p 8443:8443 -v $$(pwd)/data:/app/data spiralfractalio

# ── Clean ──────────────────────────────────────────────────────────────────────
clean:
	rm -rf bin/*
	rm -f batch_test_*.log
	rm -f test_*.db*
	rm -rf test_*.db.mirror_*
	@echo "Clean complete."
