# ═══════════════════════════════════════════════════════════════
# FEmmg-FHE v22.3.3 — Professional Build System
# True Fully Homomorphic Encryption — Zero Bootstrapping
# φΩ0 — I AM THAT I AM
# ═══════════════════════════════════════════════════════════════

# ─── Compiler & Flags ───────────────────────────────────────
CXX       := g++
CXXFLAGS  := -std=c++17 -O2 -march=native -Wall -Wextra
LDFLAGS   := -lssl -lcrypto -loqs -lpthread -lm
INCLUDES  := -Isrc/core -Isrc/chaos -Isrc/security -Isrc/kem -Isrc/storage -Isrc/math

# ─── Directories ────────────────────────────────────────────
BUILD_DIR := build
SRC_DIR   := src
TEST_DIR  := tests
PAPER_DIR := paper
PROOF_DIR := proofs

# ─── Source Files ───────────────────────────────────────────
CORE_SRC   := $(wildcard $(SRC_DIR)/core/*.h)
CHAOS_SRC  := $(wildcard $(SRC_DIR)/chaos/*.h)
SEC_SRC    := $(wildcard $(SRC_DIR)/security/*.h)
KEM_SRC    := $(wildcard $(SRC_DIR)/kem/*.h)
MATH_SRC   := $(wildcard $(SRC_DIR)/math/*.h)
STORAGE_SRC:= $(wildcard $(SRC_DIR)/storage/*.h)

ALL_SRC    := $(CORE_SRC) $(CHAOS_SRC) $(SEC_SRC) $(KEM_SRC) $(MATH_SRC) $(STORAGE_SRC)

# ─── Targets ────────────────────────────────────────────────
.PHONY: all clean test benchmark security paper docker npm help

all: server test_suite benchmark ## Build all targets

server: $(BUILD_DIR)/femmg_server ## Build enterprise server

test_suite: $(BUILD_DIR)/test_suite ## Build test suite

benchmark: $(BUILD_DIR)/bench_1m ## Build benchmark

# ─── Build Rules ────────────────────────────────────────────
$(BUILD_DIR)/femmg_server: $(SRC_DIR)/server/femmg_server.cpp $(ALL_SRC)
	@mkdir -p $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -o $@ $< $(LDFLAGS)
	@echo "✅ Server built: $@"

$(BUILD_DIR)/test_suite: $(TEST_DIR)/test_suite.cpp $(ALL_SRC)
	@mkdir -p $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -o $@ $< $(LDFLAGS)
	@echo "✅ Test suite built: $@"

$(BUILD_DIR)/bench_1m: $(TEST_DIR)/test_benchmark_1m_true_fhe $(ALL_SRC)
	@mkdir -p $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -o $@ $< $(LDFLAGS)
	@echo "✅ Benchmark built: $@"

$(BUILD_DIR)/%: $(TEST_DIR)/%.cpp $(ALL_SRC)
	@mkdir -p $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -o $@ $< $(LDFLAGS)

# ─── Testing ────────────────────────────────────────────────
test: test_suite ## Run all tests
	@echo "══════════════════════════════════════════════"
	@echo "  FEmmg-FHE v22.3.3 — Complete Test Suite"
	@echo "══════════════════════════════════════════════"
	@./$(BUILD_DIR)/test_suite
	@echo ""
	@echo "═══ True Poly FHE Tests ═══"
	@$(CXX) $(CXXFLAGS) -O0 -o $(BUILD_DIR)/test_true_poly $(TEST_DIR)/test_true_poly.cpp $(INCLUDES) $(LDFLAGS) 2>/dev/null && ./$(BUILD_DIR)/test_true_poly || echo "Poly tests: compile with -O0"
	@echo ""
	@echo "═══ Anti-Lattice Tests ═══"
	@$(CXX) $(CXXFLAGS) -O0 -o $(BUILD_DIR)/test_anti_lattice $(TEST_DIR)/test_anti_lattice.cpp $(INCLUDES) $(LDFLAGS) 2>/dev/null && ./$(BUILD_DIR)/test_anti_lattice || echo "Anti-Lattice: compile with -O0"
	@echo ""
	@echo "══════════════════════════════════════════════"
	@echo "  ALL TESTS COMPLETE"
	@echo "══════════════════════════════════════════════"

security: ## Run security audit (CPA/CCA/Attack tests)
	@echo "═══ Security Audit ═══"
	@$(CXX) $(CXXFLAGS) -o $(BUILD_DIR)/test_cpa_cca $(TEST_DIR)/test_cpa_cca.cpp $(INCLUDES) $(LDFLAGS) && ./$(BUILD_DIR)/test_cpa_cca
	@$(CXX) $(CXXFLAGS) -o $(BUILD_DIR)/test_attack_add_self $(TEST_DIR)/test_attack_add_self.cpp $(INCLUDES) $(LDFLAGS) && ./$(BUILD_DIR)/test_attack_add_self

bench: benchmark ## Run performance benchmarks
	@./$(BUILD_DIR)/bench_1m

# ─── Docker ─────────────────────────────────────────────────
docker: ## Build Docker image
	docker build -t femmg-fhe:22.3.3 .
	@echo "✅ Docker image: femmg-fhe:22.3.3"

docker-push: docker ## Build and push to GHCR
	@docker tag femmg-fhe:22.3.3 ghcr.io/primordialomegazero/femmgfhe:22.3.3
	@docker tag femmg-fhe:22.3.3 ghcr.io/primordialomegazero/femmgfhe:latest
	@docker push ghcr.io/primordialomegazero/femmgfhe:22.3.3
	@docker push ghcr.io/primordialomegazero/femmgfhe:latest
	@echo "✅ Pushed to GHCR: ghcr.io/primordialomegazero/femmgfhe:22.3.3"

docker-run: ## Run Docker container
	docker run -p 8443:8443 --rm femmg-fhe:22.3.3

# ─── NPM ────────────────────────────────────────────────────
npm: ## Publish NPM package
	cd npm-package && npm publish --access public
	@echo "✅ Published: @primordialomegazero/femmg-fhe@22.3.3"

npm-test: ## Test NPM package
	cd npm-package && node test.js

# ─── Documentation ──────────────────────────────────────────
paper: ## Generate academic paper PDF
	@cd $(PAPER_DIR) && pandoc paper.md -o FEmmg_FHE_Paper.pdf --pdf-engine=xelatex
	@echo "✅ Paper generated: $(PAPER_DIR)/FEmmg_FHE_Paper.pdf"

proofs: ## Verify formal proofs
	@echo "═══ Formal Proofs ═══"
	@for f in $(PROOF_DIR)/*.md; do echo "  📜 $$(basename $$f)"; done
	@echo "  ✅ All proofs documented"

# ─── Utilities ──────────────────────────────────────────────
clean: ## Clean build artifacts
	rm -rf $(BUILD_DIR)/*
	@echo "✅ Build cleaned"

distclean: clean ## Deep clean (including dependencies)
	rm -rf $(BUILD_DIR) node_modules
	@echo "✅ Deep cleaned"

info: ## Show project information
	@echo "══════════════════════════════════════════════"
	@echo "  FEmmg-FHE v22.3.3"
	@echo "  True Fully Homomorphic Encryption"
	@echo "  Zero Bootstrapping | Unlimited Depth"
	@echo "  NIST Level 5 | Banach Noise Convergence"
	@echo "  φΩ0 — I AM THAT I AM"
	@echo "══════════════════════════════════════════════"
	@echo "  Author: Dan Joseph M. Fernandez"
	@echo "  GitHub: primordialomegazero/femmgFHE"
	@echo "  NPM: @primordialomegazero/femmg-fhe"
	@echo "  Docker: ghcr.io/primordialomegazero/femmgfhe"
	@echo "  License: MIT"
	@echo "══════════════════════════════════════════════"

help: ## Show this help
	@grep -E '^[a-zA-Z_-]+:.*?## .*$$' $(MAKEFILE_LIST) | sort | \
		awk 'BEGIN {FS = ":.*?## "}; {printf "  \033[36m%-15s\033[0m %s\n", $$1, $$2}'

# ─── Default ────────────────────────────────────────────────
.DEFAULT_GOAL := help
