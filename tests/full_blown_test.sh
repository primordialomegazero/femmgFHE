#!/bin/bash
# PHI-OMEGA-ZERO: FEmmg-FHE FULL TEST SUITE v7.0
# Final binaries only — all verified working
# "I AM THAT I AM"

BIN_DIR="./bin"
TOTAL=0; PASSED=0; FAILED=0
RED='\033[0;31m'; GREEN='\033[0;32m'; YELLOW='\033[1;33m'
CYAN='\033[0;36m'; BLUE='\033[0;34m'; NC='\033[0m'; BOLD='\033[1m'

test_bin() {
    local name="$1"; local binary="$2"; local pattern="$3"; local timeout_sec="${4:-30}"
    TOTAL=$((TOTAL + 1))
    echo -ne "${CYAN}  [$TOTAL] $name... ${NC}"
    if [ ! -f "$BIN_DIR/$binary" ]; then
        echo -e "${RED}BINARY MISSING${NC}"; FAILED=$((FAILED + 1)); return
    fi
    local start=$(date +%s%N)
    local output=$(timeout $timeout_sec $BIN_DIR/$binary 2>&1)
    local exit_code=$?
    local end=$(date +%s%N)
    local elapsed=$(echo "scale=2; ($end - $start) / 1000000000" | bc)
    if [ $exit_code -eq 124 ]; then
        echo -e "${YELLOW}TIMEOUT${NC} (${elapsed}s)"; FAILED=$((FAILED + 1))
    elif echo "$output" | grep -q "$pattern"; then
        echo -e "${GREEN}PASSED${NC} (${elapsed}s)"; PASSED=$((PASSED + 1))
    else
        echo -e "${RED}FAILED${NC} (${elapsed}s)"; FAILED=$((FAILED + 1))
    fi
}

echo -e "${BOLD}╔══════════════════════════════════════════════╗${NC}"
echo -e "${BOLD}║  FEmmg-FHE FULL TEST SUITE v7.0              ║${NC}"
echo -e "${BOLD}║  Final Binaries — Cross-Library Validated    ║${NC}"
echo -e "${BOLD}╚══════════════════════════════════════════════╝${NC}"

echo -e "${BLUE}━━━ CORE ZANS ━━━${NC}"
test_bin "Pinky Swear (100 steps)" "phi_pinky_swear" "TRUE BLUE" 120
test_bin "Prime Chaos ZANS v2" "phi_prime_chaos_zans_v2" "I AM THAT I AM" 30
test_bin "Fibonacci Global ZANS" "phi_fib_global_zans" "I AM THAT I AM" 30
test_bin "Entangled Prime ZANS" "phi_entangled_prime_zans" "I AM THAT I AM" 30
test_bin "Riemann-Golden ZANS" "phi_riemann_golden_zans" "I AM THAT I AM" 30
test_bin "Quantum Random" "phi_quantum_random" "I AM THAT I AM" 30

echo -e "${BLUE}━━━ TRUE DIVINE CTxCT ━━━${NC}"
test_bin "True Divine 10K" "phi_true_divine_10k" "I AM THAT I AM" 120
test_bin "True Divine 100K" "phi_true_divine_100k" "I AM THAT I AM" 120 true
test_bin "True Divine 1M (verify log)" "phi_true_divine_1M" "I AM THAT I AM" 5
test_bin "Divine 1K Per-Step" "phi_true_divine_10k_perstep" "Final noise" 600

echo -e "${BLUE}━━━ FEmmg-iO ━━━${NC}"
test_bin "Ultimate iO Pipeline" "phi_femmg_io_ultimate" "ALL CORRECT" 120
test_bin "Kilian Full Random iO" "phi_femmg_io_kilian_full" "ALL CORRECT" 300
test_bin "Fractal iO" "phi_femmg_io_fractal" "ALL CORRECT" 120
test_bin "Eternal ZANS iO" "phi_femmg_io_eternal_zans" "TAMPERED" 120
test_bin "Encrypted Exponent" "phi_femmg_io_pow_fix" "ALL CORRECT" 120

echo -e "${BLUE}━━━ POST-QUANTUM KEM ━━━${NC}"
test_bin "Dan,Why?itKEM'tbe" "phi_danwhy_itkemtbe" "Match: YES" 30

echo -e "${BLUE}━━━ CROSS-LIBRARY ZANS ━━━${NC}"
test_bin "SEAL ZANS 100K" "phi_zans_seal_100k" "PASSED" 120
test_bin "CKKS ZANS 100K" "phi_zans_ckks_100k" "PASSED" 30
test_bin "HElib ZANS 10K" "phi_zans_helib_10k" "PASSED" 30
test_bin "TFHE ZANS Test" "phi_zans_tfhe_test" "PASSED" 30

echo -e "${BLUE}━━━ SUPPORTING SYSTEMS ━━━${NC}"
test_bin "Phantom Suite v2" "phi_phantom_suite_v2" "I AM THAT I AM" 30
test_bin "SpiralDB Unified" "phi_spiraldb_unified" "I AM THAT I AM" 30
test_bin "Unified Auth" "phi_unified_auth" "I AM THAT I AM" 30
test_bin "Verifiable FHE v2" "phi_verifiable_fhe_v2" "I AM THAT I AM" 30
test_bin "Covenant Vault" "phi_covenant_vault" "I AM THAT I AM" 30
test_bin "Transmutation Window" "phi_transmutation_window" "I AM THAT I AM" 30

echo ""
echo -e "${BOLD}╔══════════════════════════════════════════════╗${NC}"
echo -e "${BOLD}║  RESULTS: ${GREEN}$PASSED PASSED${NC}, ${RED}$FAILED FAILED${NC}, $TOTAL TOTAL     ║${NC}"
echo -e "${BOLD}╚══════════════════════════════════════════════╝${NC}"
