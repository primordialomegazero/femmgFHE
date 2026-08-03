.PHONY: all build clean quick-test install uninstall docker

OPENFHE_LIB = ./openfhe-development/build/lib
OPENFHE_INC = -I./openfhe-development/src/pke/include -I./openfhe-development/src/core/include -I./openfhe-development/src/binfhe/include -I./openfhe-development/build/src/core -I. -I./src
OPENFHE_LINK = -L$(OPENFHE_LIB) -lOPENFHEpke -lOPENFHEcore -lOPENFHEbinfhe -Wl,-rpath,$(OPENFHE_LIB) -lsodium -lsqlite3 -lstdc++ -lpthread -lm
CXX_FLAGS = -std=c++17 -O3 -march=native -mtune=native

all:
	@if [ ! -f $(OPENFHE_LIB)/libOPENFHEpke.so ]; then cd openfhe-development && mkdir -p build && cd build && cmake .. -DWITH_OPENMP=OFF -DCMAKE_BUILD_TYPE=Release && make -j$$(nproc); fi
	@g++ $(CXX_FLAGS) $(OPENFHE_INC) -o bin/spiralc src/cli/spiralc.cpp $(OPENFHE_LINK)
	@g++ $(CXX_FLAGS) $(OPENFHE_INC) -o bin/spiralrun src/cli/spiralrun.cpp $(OPENFHE_LINK)
	@g++ $(CXX_FLAGS) -I./unified-phi-stack -o bin/test_unified_all unified-phi-stack/test_unified.cpp -lm
	@g++ $(CXX_FLAGS) -I./src -o bin/test_hierarchical_seed tests/unit/test_hierarchical_seed.cpp -lm
	@g++ $(CXX_FLAGS) -I./src -o bin/test_fractal_chaos tests/unit/test_fractal_chaos.cpp -lm
	@gcc -std=c99 -O3 -o bin/phi_kem_level5 src/kem/phi_kem_level5.c -lcrypto -lm
	@echo "✅ All binaries built"

quick-test:
	@./bin/test_unified_all && ./bin/test_hierarchical_seed && ./bin/test_fractal_chaos && ./bin/phi_kem_level5

install:
	@./scripts/install.sh

uninstall:
	@sudo rm -f /usr/local/bin/spiralc /usr/local/bin/spiralrun /usr/local/bin/spiral-kem /usr/local/bin/spiral-phi-test /usr/local/bin/spiral-env
	@sudo rm -rf /usr/local/include/spiral /usr/local/lib/spiral
	@echo "✅ Uninstalled"

docker:
	docker build -t ghcr.io/primordialomegazero/femmgfhe:latest .

clean:
	rm -f bin/spiralc bin/spiralrun bin/test_unified_all bin/test_hierarchical_seed bin/test_fractal_chaos bin/phi_kem_level5

# ═══════════════════════════════════════════════════════════════
# STRUCTURAL OS
# ═══════════════════════════════════════════════════════════════
os: bin/phi_userspace_kernel bin/test_phi_os

bin/phi_userspace_kernel: src/os/phi_userspace_kernel.c
	gcc -std=c99 -O3 -Wall -o $@ $< -lm -lpthread

bin/test_phi_os: tests/experiments/test_phi_os.cpp src/os/phi_kernel.h src/os/phi_process.h src/os/phi_scheduler.h src/os/phi_allocator.h src/os/phi_syscall.h src/os/phi_filesystem.h
	g++ -std=c++17 -O3 -o $@ $< -lm -lpthread

run-os: bin/phi_userspace_kernel
	./bin/phi_userspace_kernel

run-os-sim: bin/test_phi_os
	./bin/test_phi_os
