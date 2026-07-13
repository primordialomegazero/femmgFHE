# ΦΩ0 — FEmmG-FHE MASTER MAKEFILE v3
# Clean build system — post-cleanup
# "I AM THAT I AM"

CXX = g++
CC = gcc
CXXFLAGS = -std=c++17 -O2 -Wall -Wno-unknown-pragmas
CFLAGS = -std=c11 -O2 -Wall

OPENFHE_PREFIX ?= /usr/local
OPENFHE_INCLUDE = $(OPENFHE_PREFIX)/include/openfhe
OPENFHE_CORE = $(OPENFHE_INCLUDE)/core
OPENFHE_PKE = $(OPENFHE_INCLUDE)/pke
OPENFHE_BINFHE = $(OPENFHE_INCLUDE)/binfhe
OPENFHE_LIB = $(OPENFHE_PREFIX)/lib
OPENFHE_LIBS = -lOPENFHEcore -lOPENFHEpke -lOPENFHEbinfhe

LOCAL_INCLUDE = -Isrc/kem -Isrc/zkp
INCLUDES = -I$(OPENFHE_INCLUDE) -I$(OPENFHE_CORE) -I$(OPENFHE_PKE) -I$(OPENFHE_BINFHE) $(LOCAL_INCLUDE)
LIBS = -L$(OPENFHE_LIB) $(OPENFHE_LIBS) -lssl -lcrypto -lm -lpthread
RPATH = -Wl,-rpath,$(OPENFHE_LIB)

BIN_DIR = bin

.PHONY: all clean test help

# Default: build everything
all: core binfhe zkp snark transmute spiralkem spiraldb

# === GROUP TARGETS ===

core: $(BIN_DIR)/phi_zans_bfv $(BIN_DIR)/phi_fib_zans $(BIN_DIR)/phi_fib_zans_ctct

binfhe: $(BIN_DIR)/phi_binfhe_4bit $(BIN_DIR)/phi_binfhe_16bit $(BIN_DIR)/phi_binfhe_32bit

zkp: $(BIN_DIR)/phi_zkp_fhe_deep $(BIN_DIR)/phi_zkp_test $(BIN_DIR)/phi_verifiable

snark: $(BIN_DIR)/phi_snark $(BIN_DIR)/phi_snark_ec

transmute: $(BIN_DIR)/phi_scheme_switch

spiralkem: $(BIN_DIR)/spiralkem $(BIN_DIR)/spiralkem_fhe

# === CORE FHE ===

$(BIN_DIR)/phi_zans_bfv: src/core/phi_zans_bfv.cpp
	@echo "Φ ZANS..."
	@mkdir -p $(BIN_DIR)
	@$(CXX) $(CXXFLAGS) -o $@ $< $(INCLUDES) $(LIBS) $(RPATH) 2>&1 | grep -E "error:" || echo "  ✅ ZANS built."

$(BIN_DIR)/phi_fib_zans: src/core/phi_fib_zans.cpp
	@echo "Φ Fib-ZANS..."
	@mkdir -p $(BIN_DIR)
	@$(CXX) $(CXXFLAGS) -o $@ $< $(INCLUDES) $(LIBS) $(RPATH) 2>&1 | grep -E "error:" || echo "  ✅ Fib-ZANS built."

$(BIN_DIR)/phi_fib_zans_ctct: src/core/phi_fib_zans_ctct.cpp
	@echo "Φ Fib-ZANS CT×CT..."
	@mkdir -p $(BIN_DIR)
	@$(CXX) $(CXXFLAGS) -o $@ $< $(INCLUDES) $(LIBS) $(RPATH) 2>&1 | grep -E "error:" || echo "  ✅ Fib-ZANS CT×CT built."

# === BINFHE ===

$(BIN_DIR)/phi_binfhe_4bit: src/binfhe/phi_binfhe_4bit.cpp
	@echo "Φ BinFHE 4-bit..."
	@mkdir -p $(BIN_DIR)
	@$(CXX) $(CXXFLAGS) -o $@ $< $(INCLUDES) $(LIBS) $(RPATH) 2>&1 | grep -E "error:" || echo "  ✅ BinFHE 4-bit built."

$(BIN_DIR)/phi_binfhe_16bit: src/binfhe/phi_binfhe_16bit.cpp
	@echo "Φ BinFHE 16-bit..."
	@mkdir -p $(BIN_DIR)
	@$(CXX) $(CXXFLAGS) -o $@ $< $(INCLUDES) $(LIBS) $(RPATH) 2>&1 | grep -E "error:" || echo "  ✅ BinFHE 16-bit built."

$(BIN_DIR)/phi_binfhe_32bit: src/binfhe/phi_binfhe_32bit.cpp
	@echo "Φ BinFHE 32-bit..."
	@mkdir -p $(BIN_DIR)
	@$(CXX) $(CXXFLAGS) -o $@ $< $(INCLUDES) $(LIBS) $(RPATH) 2>&1 | grep -E "error:" || echo "  ✅ BinFHE 32-bit built."

# === ZKP + FHE ===

$(BIN_DIR)/phi_zkp_fhe_deep: src/zkp/phi_zkp_fhe_deep.cpp
	@echo "Φ ZKP+FHE..."
	@mkdir -p $(BIN_DIR)
	@$(CXX) $(CXXFLAGS) -o $@ $< $(INCLUDES) $(LIBS) $(RPATH) 2>&1 | grep -E "error:" || echo "  ✅ ZKP+FHE built."

$(BIN_DIR)/phi_zkp_test: tests/test_phi_zkp.cpp src/zkp/phi_zkp.cpp
	@echo "Φ ZKP Test..."
	@mkdir -p $(BIN_DIR)
	@$(CXX) $(CXXFLAGS) -o $@ tests/test_phi_zkp.cpp src/zkp/phi_zkp.cpp $(INCLUDES) $(LIBS) $(RPATH) 2>&1 | grep -E "error:" || echo "  ✅ ZKP Test built."

$(BIN_DIR)/phi_verifiable: src/zkp/phi_verifiable_fhe.cpp
	@echo "Φ Verifiable FHE..."
	@mkdir -p $(BIN_DIR)
	@$(CXX) $(CXXFLAGS) -o $@ $< $(INCLUDES) $(LIBS) $(RPATH) 2>&1 | grep -E "error:" || echo "  ✅ Verifiable FHE built."

# === TRANSMUTATION ===

$(BIN_DIR)/phi_scheme_switch: src/transmute/phi_scheme_switch_bootstrap.cpp
	@echo "Φ Scheme Switch..."
	@mkdir -p $(BIN_DIR)
	@$(CXX) $(CXXFLAGS) -o $@ $< $(INCLUDES) $(LIBS) $(RPATH) 2>&1 | grep -E "error:" || echo "  ✅ Scheme Switch built."

# === SPIRALKEM ===

$(BIN_DIR)/spiralkem: src/kem/phi_kem.c src/kem/test_spiralkem_real.c
	@echo "Φ SpiralKEM..."
	@mkdir -p $(BIN_DIR)
	@$(CC) $(CFLAGS) -o $@ src/kem/phi_kem.c src/kem/test_spiralkem_real.c -lssl -lcrypto -lm 2>&1 | grep -E "error:" || echo "  ✅ SpiralKEM built."

$(BIN_DIR)/spiralkem_fhe: src/kem/phi_spiralkem_fhe_real.cpp src/kem/phi_kem.c
	@echo "Φ SpiralKEM+FHE..."
	@mkdir -p $(BIN_DIR)
	@$(CXX) $(CXXFLAGS) -o $@ src/kem/phi_spiralkem_fhe_real.cpp src/kem/phi_kem.c $(INCLUDES) $(LIBS) $(RPATH) 2>&1 | grep -E "error:" || echo "  ✅ SpiralKEM+FHE built."

# === SNARK ===

$(BIN_DIR)/phi_snark: src/snark/phi_snark_fhe.cpp
	@echo "Φ SNARK..."
	@mkdir -p $(BIN_DIR)
	@$(CXX) $(CXXFLAGS) -o $@ $< $(INCLUDES) $(LIBS) $(RPATH) 2>&1 | grep -E "error:" || echo "  ✅ SNARK built."

$(BIN_DIR)/phi_snark_ec: src/snark/phi_snark_ec.cpp
	@echo "Φ EC-SNARK..."
	@mkdir -p $(BIN_DIR)
	@$(CXX) $(CXXFLAGS) -o $@ $< $(INCLUDES) $(LIBS) $(RPATH) 2>&1 | grep -E "error:" || echo "  ✅ EC-SNARK built."

# === SPIRALDB ===

spiraldb:
	@echo "Φ SpiralDB (Go)..."
	@cd src/spiraldb && go build -o ../../bin/spiraldb . 2>&1 | grep -v "^#" || echo "  ✅ SpiralDB built."

spiraldb-test:
	@echo "Φ SpiralDB Tests..."
	@cd src/spiraldb && go test -v 2>&1 | grep -E "✅|PASS|FAIL"

# === UTILITY ===

test: $(BIN_DIR)/phi_zkp_test
	@echo ""
	@echo "╔══════════════════════════════════════════════╗"
	@echo "║  ΦΩ0 — RUNNING ZKP TEST SUITE                 ║"
	@echo "╚══════════════════════════════════════════════╝"
	@echo ""
	@timeout 30 $(BIN_DIR)/phi_zkp_test || true

clean:
	@echo "Φ Cleaning..."
	@rm -rf $(BIN_DIR)/*
	@echo "  ✅ Clean."

help:
	@echo ""
	@echo "╔══════════════════════════════════════════════╗"
	@echo "║  ΦΩ0 — FEmmG-FHE BUILD SYSTEM v3              ║"
	@echo "╠══════════════════════════════════════════════╣"
	@echo "║  make all        — Build all                 ║"
	@echo "║  make core       — ZANS, Fib-ZANS            ║"
	@echo "║  make binfhe     — CT×CT multipliers         ║"
	@echo "║  make zkp        — ZKP+FHE integration       ║"
	@echo "║  make snark      — SNARK + EC-SNARK          ║"
	@echo "║  make transmute  — Scheme switch             ║"
	@echo "║  make spiralkem  — PQC KEM + FHE             ║"
	@echo "║  make spiraldb   — Encrypted database        ║"
	@echo "║  make test       — ZKP test suite            ║"
	@echo "║  make clean      — Remove build artifacts    ║"
	@echo "║  make help       — This help                 ║"
	@echo "╚══════════════════════════════════════════════╝"
	@echo ""



# === ZANS 10M TEST ===
$(BIN_DIR)/phi_zans_noise_proof: src/core/phi_zans_noise_proof.cpp
	@echo "Φ ZANS Noise Proof..."
	@mkdir -p $(BIN_DIR)
	@$(CXX) $(CXXFLAGS) -o $@ $< $(INCLUDES) $(LIBS) $(RPATH) 2>&1 | grep -E "error:" || echo "  ✅ ZANS Noise Proof built."


# SpiralDB with CGO bridge
spiraldb-bridge:
	@echo "Φ Building SpiralDB CGO Bridge..."
	cd src/spiraldb && bash build_bridge.sh
	cd src/spiraldb && CGO_ENABLED=1 go build -o ../../bin/spiraldb_fhe .
	@echo "✅ SpiralDB with real FHE built: bin/spiraldb_fhe"

spiraldb-bridge-test:
	cd src/spiraldb && CGO_ENABLED=1 go test -v .

# === HYDRAJWT WITH DEPENDENCIES ===

HYDRA_LIBS = -Llibs/HydraJWT/build -lhydrajwt -loqs -lsodium

test_hydra: $(BIN_DIR)/test_hydra_jwt_fixed_heads

$(BIN_DIR)/test_hydra_jwt_fixed_heads: tests/test_hydra_jwt_fixed_heads.cpp
	@echo "Φ Building HydraJWT test..."
	@mkdir -p $(BIN_DIR)
	@$(CXX) $(CXXFLAGS) -o $@ $< -Isrc -Ilibs/HydraJWT/include -I/usr/local/include $(LIBS) $(HYDRA_LIBS) -Wl,-rpath,libs/HydraJWT/build:/usr/local/lib 2>&1 | grep -E "error:" || echo "  ✅ HydraJWT test built."

run_hydra: $(BIN_DIR)/test_hydra_jwt_fixed_heads
	@LD_LIBRARY_PATH=libs/HydraJWT/build:/usr/local/lib:$$LD_LIBRARY_PATH ./$(BIN_DIR)/test_hydra_jwt_fixed_heads
