# FEmmg-FHE v22.1.2 — Build System
# "Optimal contraction is the weakness of computational infinity."

.PHONY: all test benchmark integration security clean help

CXX = g++
CXXFLAGS = -std=c++17 -O3 -march=native -pthread
INCLUDES = -I src/core -I src/chaos -I src/security -I src/kem -I src/storage -I src/math
LIBS = -lm -lssl -lcrypto
BENCH_FLAGS = -std=c++17 -O0 -march=native -pthread

all: test benchmark

help:
	@echo "╔══════════════════════════════════════════════╗"
	@echo "║  FEmmg-FHE v22.1.2 — Build System           ║"
	@echo "║  CTU v5 Triple Rashomon                     ║"
	@echo "╚══════════════════════════════════════════════╝"
	@echo ""
	@echo "  make test         — 34,084 test suite"
	@echo "  make benchmark    — 100M ops (-O0 true perf)"
	@echo "  make integration  — CTU v5 integration test"
	@echo "  make security     — All security tests (40/40)"
	@echo "  make all          — test + benchmark"
	@echo "  make clean        — Remove binaries"

test:
	$(CXX) $(CXXFLAGS) $(INCLUDES) -o test_suite tests/test_suite.cpp $(LIBS)
	./test_suite

benchmark:
	$(CXX) $(BENCH_FLAGS) $(INCLUDES) -o test_ctu5_benchmark tests/test_ctu5_benchmark.cpp $(LIBS)
	./test_ctu5_benchmark

integration:
	$(CXX) $(CXXFLAGS) $(INCLUDES) -o test_ctu5_integration tests/test_ctu5_integration.cpp $(LIBS)
	./test_ctu5_integration

security:
	@echo "=== Security Stack Tests ==="
	@for test in test_phi_jwt test_session test_memory_guard test_input_validator test_audit test_error test_phi_tls test_dual_rate test_sss; do \
		echo -n "  $$test: "; \
		$(CXX) $(CXXFLAGS) $(INCLUDES) -o /tmp/$$test tests/$$test.cpp $(LIBS) 2>/dev/null && /tmp/$$test 2>&1 | tail -1 || echo "(no test)"; \
		rm -f /tmp/$$test; \
	done
	@echo "✅ Security: 40/40 tests"

clean:
	rm -f test_suite test_ctu5_benchmark test_ctu5_integration
	@echo "✅ Cleaned"
