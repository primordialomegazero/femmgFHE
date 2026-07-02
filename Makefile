# FEmmg-FHE v22.3 — Makefile
# PHI-OMEGA-ZERO — I AM THAT I AM

CXX = g++
CXXFLAGS = -std=c++17 -O0 -march=native -pthread
INCLUDES = -I src/core -I src/chaos -I src/security -I src/kem -I src/storage -I src/math
LIBS = -lm -lssl -lcrypto

SRC_DIR = src
TEST_DIR = tests
BUILD_DIR = build

# ── Sources ──────────────────────────────────
CHAOS_SRC = $(SRC_DIR)/chaos/triple_rashomon.h \
            $(SRC_DIR)/chaos/void_engine.h \
            $(SRC_DIR)/chaos/phi_irrational_nonce.h \
            $(SRC_DIR)/chaos/golden_chaos.h \
            $(SRC_DIR)/chaos/fibonacci_duel.h

CORE_SRC = $(SRC_DIR)/core/banach_engine.h \
           $(SRC_DIR)/core/femmg_operations.h \
           $(SRC_DIR)/core/phi_stack.h \
           $(SRC_DIR)/core/metaprogram.h

SECURITY_SRC = $(SRC_DIR)/security/memory_guard.h \
               $(SRC_DIR)/security/blackhole_active.h \
               $(SRC_DIR)/security/time_manipulator.h \
               $(SRC_DIR)/security/dual_rate_limiter.h \
               $(SRC_DIR)/security/zkp_pqc.h \
               $(SRC_DIR)/security/zkp_groth16.h \
               $(SRC_DIR)/security/phi_jwt.h \
               $(SRC_DIR)/security/phi_tls.h \
               $(SRC_DIR)/security/security.h \
               $(SRC_DIR)/security/security_complete.h \
               $(SRC_DIR)/security/guardian.h \
               $(SRC_DIR)/security/anti_matter_v2.h \
               $(SRC_DIR)/security/audit_log.h \
               $(SRC_DIR)/security/blackhole.h \
               $(SRC_DIR)/security/blackhole_history.h \
               $(SRC_DIR)/security/error_handler.h \
               $(SRC_DIR)/security/input_validator.h \
               $(SRC_DIR)/security/session_manager.h \
               $(SRC_DIR)/security/sss_error_handler.h \
               $(SRC_DIR)/security/zkp_fractal.h

KEM_SRC = $(SRC_DIR)/kem/phi_parallel_kem.h \
          $(SRC_DIR)/kem/phi_algo_merge.h \
          $(SRC_DIR)/kem/ml_kem_wrapper.h

MATH_SRC = $(SRC_DIR)/math/phi_constants.h \
           $(SRC_DIR)/math/riemann_chaos.h \
           $(SRC_DIR)/math/riemann_deep.h \
           $(SRC_DIR)/math/riemann_zeros.h \
           $(SRC_DIR)/math/riemann_zeta.h

STORAGE_SRC = $(SRC_DIR)/storage/spiral_db_lite.h

SERVER_SRC = $(SRC_DIR)/server/femmg_server.cpp \
             $(SRC_DIR)/server/tls_wrapper.h

ALL_HEADERS = $(CHAOS_SRC) $(CORE_SRC) $(SECURITY_SRC) $(KEM_SRC) $(MATH_SRC) $(STORAGE_SRC)

# ── Targets ──────────────────────────────────
.PHONY: all server test clean

all: server

server: $(SERVER_SRC) $(ALL_HEADERS)
	@mkdir -p $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -o $(BUILD_DIR)/femmg_server $(SRC_DIR)/server/femmg_server.cpp $(LIBS)
	@echo "✅ Server compiled: build/femmg_server"

# ── Tests ────────────────────────────────────
test_true_fhe: $(TEST_DIR)/test_true_fhe.cpp $(ALL_HEADERS)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -o $(BUILD_DIR)/$@ $< $(LIBS)
	@./$(BUILD_DIR)/$@

test_cpa_cca: $(TEST_DIR)/test_cpa_cca.cpp $(ALL_HEADERS)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -o $(BUILD_DIR)/$@ $< $(LIBS)
	@./$(BUILD_DIR)/$@

test_fractal: $(TEST_DIR)/test_fractal_fhe_full.cpp $(ALL_HEADERS)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -o $(BUILD_DIR)/$@ $< $(LIBS)
	@./$(BUILD_DIR)/$@

test_benchmark_1m: $(TEST_DIR)/test_benchmark_1m.cpp $(ALL_HEADERS)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -o $(BUILD_DIR)/$@ $< $(LIBS)
	@./$(BUILD_DIR)/$@

test_benchmark_1m_fractal: $(TEST_DIR)/test_benchmark_1m_fractal.cpp $(ALL_HEADERS)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -o $(BUILD_DIR)/$@ $< $(LIBS)
	@./$(BUILD_DIR)/$@

test_benchmark_10m: $(TEST_DIR)/test_benchmark_10m.cpp $(ALL_HEADERS)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -o $(BUILD_DIR)/$@ $< $(LIBS)
	@./$(BUILD_DIR)/$@

test_benchmark_100m: $(TEST_DIR)/test_benchmark_100m.cpp $(ALL_HEADERS)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -o $(BUILD_DIR)/$@ $< $(LIBS)
	@./$(BUILD_DIR)/$@

test_void: $(TEST_DIR)/test_void_engine.cpp $(ALL_HEADERS)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -o $(BUILD_DIR)/$@ $< $(LIBS)
	@./$(BUILD_DIR)/$@

test_ml_kem: $(TEST_DIR)/test_ml_kem.cpp $(ALL_HEADERS)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -o $(BUILD_DIR)/$@ $< $(LIBS)
	@./$(BUILD_DIR)/$@

test_groth16: $(TEST_DIR)/test_groth16.cpp $(ALL_HEADERS)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -o $(BUILD_DIR)/$@ $< $(LIBS)
	@./$(BUILD_DIR)/$@

test_zkp: $(TEST_DIR)/test_zkp_all.cpp $(ALL_HEADERS)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -o $(BUILD_DIR)/$@ $< $(LIBS)
	@./$(BUILD_DIR)/$@

test_smart: $(TEST_DIR)/test_smart_fhe.cpp $(ALL_HEADERS)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -o $(BUILD_DIR)/$@ $< $(LIBS)
	@./$(BUILD_DIR)/$@

test_suite: $(TEST_DIR)/test_suite.cpp $(ALL_HEADERS)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -o $(BUILD_DIR)/$@ $< $(LIBS)
	@./$(BUILD_DIR)/$@

test_all: test_true_fhe test_cpa_cca test_fractal test_void test_ml_kem test_groth16 test_zkp test_smart
	@echo ""
	@echo "======================================================"
	@echo "  ALL TESTS COMPLETE"
	@echo "======================================================"

# ── Benchmarks ───────────────────────────────
bench_1m: test_benchmark_1m
bench_1m_fractal: test_benchmark_1m_fractal
bench_10m: test_benchmark_10m
bench_100m: test_benchmark_100m

# ── Clean ────────────────────────────────────
clean:
	rm -rf $(BUILD_DIR)/*
	@echo "✅ Build directory cleaned"

distclean: clean
	rm -f femmg_server test_true_fhe test_cpa_cca
	@echo "✅ All binaries removed"

# ── Info ─────────────────────────────────────
info:
	@echo "======================================================"
	@echo "  FEmmg-FHE v22.3 — Build System"
	@echo "======================================================"
	@echo "  make server         Build production server"
	@echo "  make test_all       Run all tests"
	@echo "  make bench_1m       1M regular benchmark"
	@echo "  make bench_1m_fractal 1M fractal benchmark"
	@echo "  make bench_10m      10M regular benchmark"
	@echo "  make bench_100m     100M regular benchmark"
	@echo "  make clean          Clean build artifacts"
	@echo "======================================================"
