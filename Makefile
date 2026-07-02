# ═══════════════════════════════════════════════════════════════
# FEmmg-FHE v23.0.2 — Professional Build System
# True Fully Homomorphic Encryption — Zero Bootstrapping
# Lyapunov-Stabilized Floating-Point FHE
# φΩ0 — I AM THAT I AM
# ═══════════════════════════════════════════════════════════════

# ─── Compiler & Flags ───────────────────────────────────────
CXX       := g++
CXXFLAGS  := -std=c++17 -O2 -march=native -Wall -Wextra
CXXFLAGS0 := -std=c++17 -O0
LDFLAGS   := -lssl -lcrypto -loqs -lpthread -lm
INCLUDES  := -Isrc/core -Isrc/chaos -Isrc/security -Isrc/kem -Isrc/storage -Isrc/math

# ─── Directories ────────────────────────────────────────────
BUILD_DIR := build
SRC_DIR   := src
TEST_DIR  := tests
DOCS_DIR  := docs

# ─── Source Files ───────────────────────────────────────────
ALL_SRC := $(wildcard $(SRC_DIR)/*/*.h)

# ─── Targets ────────────────────────────────────────────────
.PHONY: all clean test benchmark security docker help

all: server test_suite ## Build all targets

server: $(BUILD_DIR)/femmg_server ## Build enterprise server

test_suite: $(BUILD_DIR)/test_suite ## Build test suite

benchmark: $(BUILD_DIR)/test_benchmark ## Build benchmark

# ─── Build Rules ────────────────────────────────────────────
$(BUILD_DIR)/femmg_server: $(SRC_DIR)/server/femmg_server.cpp $(ALL_SRC)
	@mkdir -p $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -o $@ $< $(LDFLAGS)
	@echo "✅ Server built: $@"

$(BUILD_DIR)/test_suite: $(TEST_DIR)/test_suite.cpp $(ALL_SRC)
	@mkdir -p $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -o $@ $< $(LDFLAGS)
	@echo "✅ Test suite built: $@"

$(BUILD_DIR)/test_benchmark: $(TEST_DIR)/test_benchmark.cpp $(ALL_SRC)
	@mkdir -p $(BUILD_DIR)
	$(CXX) $(CXXFLAGS0) $(INCLUDES) -o $@ $< $(LDFLAGS)
	@echo "✅ Benchmark built: $@"

# ─── Testing ────────────────────────────────────────────────
test: test_suite ## Run core test suite
	@echo "══════════════════════════════════════════════"
	@echo "  FEmmg-FHE v23.0.2 — Core Test Suite"
	@echo "══════════════════════════════════════════════"
	@./$(BUILD_DIR)/test_suite

test-all: ## Run full test suite with report
	@./run_full_test_suite.sh

test-poly: ## Run True Poly FHE tests
	@$(CXX) $(CXXFLAGS0) -o $(BUILD_DIR)/test_true_poly $(TEST_DIR)/test_true_poly.cpp $(INCLUDES) $(LDFLAGS) && ./$(BUILD_DIR)/test_true_poly

test-lyapunov: ## Run LyapunovFHE tests
	@$(CXX) $(CXXFLAGS0) -o $(BUILD_DIR)/test_lyapunov $(TEST_DIR)/test_lyapunov.cpp $(INCLUDES) $(LDFLAGS) && ./$(BUILD_DIR)/test_lyapunov

test-security: ## Run security audit
	@$(CXX) $(CXXFLAGS0) -o $(BUILD_DIR)/test_security_audit_v2 $(TEST_DIR)/test_security_audit_v2.cpp $(INCLUDES) $(LDFLAGS) && ./$(BUILD_DIR)/test_security_audit_v2

test-zkp: ## Run ZKP tests
	@$(CXX) $(CXXFLAGS0) -o $(BUILD_DIR)/test_zkp_final $(TEST_DIR)/test_zkp_final.cpp $(INCLUDES) $(LDFLAGS) && ./$(BUILD_DIR)/test_zkp_final

test-nonce: ## Run nonce uniqueness tests
	@$(CXX) $(CXXFLAGS0) -o $(BUILD_DIR)/test_nonce_simple $(TEST_DIR)/test_nonce_simple.cpp $(INCLUDES) $(LDFLAGS) && ./$(BUILD_DIR)/test_nonce_simple

test-monster: ## Run edge case tests
	@$(CXX) $(CXXFLAGS0) -o $(BUILD_DIR)/test_monster_hunt_v2 $(TEST_DIR)/test_monster_hunt_v2.cpp $(INCLUDES) $(LDFLAGS) && ./$(BUILD_DIR)/test_monster_hunt_v2

# ─── Benchmarks ─────────────────────────────────────────────
bench: benchmark ## Run performance benchmarks
	@./$(BUILD_DIR)/test_benchmark

# ─── Docker ─────────────────────────────────────────────────
docker: ## Build Docker image
	docker build -t femmg-fhe:23.0.2 .
	@echo "✅ Docker image: femmg-fhe:23.0.2"

docker-push: docker ## Build and push to GHCR
	@docker tag femmg-fhe:23.0.2 ghcr.io/primordialomegazero/femmgfhe:23.0.2
	@docker tag femmg-fhe:23.0.2 ghcr.io/primordialomegazero/femmgfhe:latest
	@docker push ghcr.io/primordialomegazero/femmgfhe:23.0.2
	@docker push ghcr.io/primordialomegazero/femmgfhe:latest
	@echo "✅ Pushed to GHCR"

docker-run: ## Run Docker container
	docker run -p 8443:8443 --rm femmg-fhe:23.0.2

# ─── NPM ────────────────────────────────────────────────────
npm: ## Publish NPM package
	cd npm-package && npm publish --access public
	@echo "✅ Published: @primordialomegazero/femmg-fhe@23.0.2"

# ─── Documentation ──────────────────────────────────────────
docs: ## Show documentation
	@echo "═══ Documentation ═══"
	@echo "  API Reference:  $(DOCS_DIR)/api/lyapunov_fhe_api.md"
	@echo "  Formal Proofs:  $(DOCS_DIR)/proofs/main_theorems.md"
	@echo "  Security Model: $(DOCS_DIR)/proofs/security_model.md"
	@echo "  Contributing:   CONTRIBUTING.md"
	@echo "  Test Results:   $(DOCS_DIR)/test_results/test_report_latest.md"

# ─── Utilities ──────────────────────────────────────────────
clean: ## Clean build artifacts
	rm -rf $(BUILD_DIR)/*
	@echo "✅ Build cleaned"

info: ## Show project information
	@echo "══════════════════════════════════════════════"
	@echo "  FEmmg-FHE v23.0.2"
	@echo "  Lyapunov-Stabilized Floating-Point FHE"
	@echo "  Zero Bootstrapping | Unlimited Depth"
	@echo "  IEEE 754 Range | 53-bit Precision"
	@echo "  φΩ0 — I AM THAT I AM"
	@echo "══════════════════════════════════════════════"
	@echo "  Author: Dan Joseph M. Fernandez"
	@echo "  GitHub: primordialomegazero/femmgFHE"
	@echo "  License: MIT"
	@echo "══════════════════════════════════════════════"

help: ## Show this help
	@grep -E '^[a-zA-Z_-]+:.*?## .*$$' $(MAKEFILE_LIST) | sort | \
		awk 'BEGIN {FS = ":.*?## "}; {printf "  \033[36m%-18s\033[0m %s\n", $$1, $$2}'

# ─── Default ────────────────────────────────────────────────
.DEFAULT_GOAL := help
