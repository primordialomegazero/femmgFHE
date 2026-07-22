#!/bin/bash
echo "============================================"
echo "  FEmmG-FHE Repository Cleanup"
echo "============================================"
echo ""

# Create new structure
mkdir -p src/femmg
mkdir -p tests/active
mkdir -p tests/archive
mkdir -p docs
mkdir -p paper
mkdir -p archive/legacy

# ==========================================
# STEP 1: Move active test files
# ==========================================
echo "[1/5] Organizing active tests..."

ACTIVE_TESTS=(
    "test_phi_complete.cpp"
    "test_phi_clean_cycle.cpp"
    "test_phi_ckks_trace.cpp"
    "test_phi_ckks_ultra.cpp"
    "test_phi_vs_bootstrap.cpp"
    "test_phi_stress.cpp"
    "test_phi_prescale.cpp"
    "test_phi_32768.cpp"
    "test_phi_bootfree_v2.cpp"
    "test_phi_deep_lite.cpp"
    "test_phi_gauntlet.cpp"
    "test_phi_unlimited.cpp"
    "test_phi_sprint.cpp"
    "test_phi_asymmetric_clean.cpp"
    "test_phi_pure_mulx.cpp"
    "test_phi_binet.cpp"
    "test_snc_verify.cpp"
    "test_phi_final.cpp"
)

for f in "${ACTIVE_TESTS[@]}"; do
    if [ -f "tests/$f" ]; then
        cp "tests/$f" "tests/active/$f"
        echo "  ✓ $f"
    fi
done

# ==========================================
# STEP 2: Move all other tests to archive
# ==========================================
echo "[2/5] Archiving old tests..."

for f in tests/*.cpp tests/experiments/*.cpp tests/outputs/*.cpp; do
    if [ -f "$f" ]; then
        basename=$(basename "$f")
        if [ ! -f "tests/active/$basename" ]; then
            mv "$f" "tests/archive/$basename" 2>/dev/null
        fi
    fi
done

# ==========================================
# STEP 3: Clean root directory
# ==========================================
echo "[3/5] Cleaning root directory..."

# Keep only essential files in root
ESSENTIAL_ROOT=(
    "Makefile"
    "README.md"
    "LICENSE"
    ".gitignore"
    "cleanup_repo.sh"
)

for f in *; do
    if [ -f "$f" ] && [[ ! " ${ESSENTIAL_ROOT[@]} " =~ " $f " ]]; then
        # Skip important dirs
        if [ "$f" != "src" ] && [ "$f" != "tests" ] && [ "$f" != "docs" ] && \
           [ "$f" != "paper" ] && [ "$f" != "archive" ] && [ "$f" != "openfhe-development" ] && \
           [ "$f" != "SEAL" ] && [ "$f" != "libs" ] && [ "$f" != "bindings" ]; then
            mv "$f" "archive/legacy/$f" 2>/dev/null
        fi
    fi
done

# ==========================================
# STEP 4: Create core library header
# ==========================================
echo "[4/5] Creating core library..."

cat > src/femmg/phi_core.h << 'EOF'
// FEmmG-FHE Core Library
// φ-extension ring: R[X]/(X²-X-1) for FHE noise management
// Author: Dan Joseph M. Fernandez / Primordial Omega Zero
// License: MIT

#pragma once
#include <openfhe.h>
#include <vector>
#include <cmath>

namespace femmg {

const double PHI = 1.6180339887498948482;
const double PSI = -0.6180339887498948482;

struct PhiElement {
    lbcrypto::Ciphertext<lbcrypto::DCRTPoly> a;  // φ-coefficient
    lbcrypto::Ciphertext<lbcrypto::DCRTPoly> b;  // ψ-coefficient
};

// Core operations
PhiElement mul_X(lbcrypto::CryptoContext<lbcrypto::DCRTPoly>& cc, const PhiElement& x);
PhiElement div_X(lbcrypto::CryptoContext<lbcrypto::DCRTPoly>& cc, const PhiElement& x);
PhiElement clean(lbcrypto::CryptoContext<lbcrypto::DCRTPoly>& cc, const PhiElement& x, int ratio = 3);
PhiElement multiply(lbcrypto::CryptoContext<lbcrypto::DCRTPoly>& cc, const PhiElement& x, const PhiElement& y);
PhiElement bootstrap(lbcrypto::CryptoContext<lbcrypto::DCRTPoly>& cc, 
                     lbcrypto::KeyPair<lbcrypto::DCRTPoly>& keys,
                     const PhiElement& x, uint32_t slots);

// Utility
double decrypt_coeff(lbcrypto::CryptoContext<lbcrypto::DCRTPoly>& cc,
                     lbcrypto::KeyPair<lbcrypto::DCRTPoly>& keys,
                     const lbcrypto::Ciphertext<lbcrypto::DCRTPoly>& ct, uint32_t slots);
PhiElement encrypt_element(lbcrypto::CryptoContext<lbcrypto::DCRTPoly>& cc,
                           lbcrypto::KeyPair<lbcrypto::DCRTPoly>& keys,
                           double a, double b, uint32_t slots);
double get_phi_value(const PhiElement& x, lbcrypto::CryptoContext<lbcrypto::DCRTPoly>& cc,
                     lbcrypto::KeyPair<lbcrypto::DCRTPoly>& keys, uint32_t slots);
double get_psi_noise(const PhiElement& x, lbcrypto::CryptoContext<lbcrypto::DCRTPoly>& cc,
                     lbcrypto::KeyPair<lbcrypto::DCRTPoly>& keys, uint32_t slots);

} // namespace femmg
EOF

echo "  ✓ src/femmg/phi_core.h created"

# ==========================================
# STEP 5: Update Makefile
# ==========================================
echo "[5/5] Updating Makefile..."

cat > Makefile << 'EOF'
# FEmmG-FHE Makefile
CXX = g++
CXXFLAGS = -std=c++17 -O3 -march=native
INCLUDES = -I./openfhe-development/src/pke/include \
           -I./openfhe-development/src/core/include \
           -I./openfhe-development/src/binfhe/include \
           -I./openfhe-development/build/src/core \
           -I./src
LIBS = -L./openfhe-development/build/lib \
       -lOPENFHEpke -lOPENFHEcore -lOPENFHEbinfhe \
       -Wl,-rpath,./openfhe-development/build/lib \
       -lstdc++ -lpthread -lm

# Active tests (from tests/active/)
TESTS = test_phi_complete test_phi_clean_cycle test_phi_ckks_trace \
        test_phi_ckks_ultra test_phi_vs_bootstrap test_phi_stress \
        test_phi_prescale test_phi_32768 test_phi_bootfree_v2 \
        test_phi_deep_lite test_phi_gauntlet test_phi_unlimited \
        test_phi_sprint test_phi_asymmetric_clean test_phi_pure_mulx \
        test_phi_binet test_snc_verify test_phi_final

.PHONY: all clean

all: $(TESTS)

define TEST_TEMPLATE
$(1): tests/active/$(1).cpp
	$(CXX) $(CXXFLAGS) $(INCLUDES) -o bin/$(1) tests/active/$(1).cpp $(LIBS)
endef

$(foreach t,$(TESTS),$(eval $(call TEST_TEMPLATE,$(t))))

clean:
	rm -f bin/*
	rm -f gauntlet_log.txt
	rm -f core*

distclean: clean
	rm -rf openfhe-development/build/*
EOF

echo "  ✓ Makefile updated"

echo ""
echo "============================================"
echo "  CLEANUP COMPLETE"
echo "============================================"
echo ""
echo "  Active tests: tests/active/"
echo "  Archived:     tests/archive/ + archive/"
echo "  Core library: src/femmg/phi_core.h"
echo "  Docs:         docs/"
echo "  Paper:        paper/"
echo ""
echo "  Run 'make all' to build active tests"
echo "============================================"
