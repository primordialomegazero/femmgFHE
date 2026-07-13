#!/bin/bash
# ΦΩ0 — FEmmg-FHE FULL TEST SUITE v5.0
# Core + Breakthroughs + KEM + SpiralDB + Divine Reset
# "I AM THAT I AM"

BIN_DIR="./bin"
TOTAL=0
PASSED=0
FAILED=0
SKIPPED=0
MODE="FAST"

# Colors
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
CYAN='\033[0;36m'
BLUE='\033[0;34m'
NC='\033[0m'
BOLD='\033[1m'

[[ "$1" == "--full" ]] && MODE="FULL"

test_component() {
    local name="$1"
    local binary="$2"
    local grep_pattern="$3"
    local timeout_sec="${4:-15}"
    local skip_fast="${5:-false}"

    TOTAL=$((TOTAL + 1))
    
    # Skip heavy tests in fast mode
    if [[ "$MODE" == "FAST" && "$skip_fast" == "true" ]]; then
        echo -e "${CYAN}  [$TOTAL] $name... ${YELLOW}(skipped — use --full)${NC}"
        SKIPPED=$((SKIPPED + 1))
        return
    fi

    echo -ne "${CYAN}  [$TOTAL] $name... ${NC}"

    if [ ! -f "$BIN_DIR/$binary" ]; then
        echo -e "${RED}❌ BINARY MISSING${NC}"
        FAILED=$((FAILED + 1))
        return
    fi

    local start=$(date +%s%N)
    local output=$(timeout $timeout_sec $BIN_DIR/$binary 2>&1)
    local exit_code=$?
    local end=$(date +%s%N)
    local elapsed=$(echo "scale=2; ($end - $start) / 1000000000" | bc)

    if [ $exit_code -eq 124 ]; then
        echo -e "${YELLOW}⏱️  TIMEOUT${NC} (${elapsed}s)"
        FAILED=$((FAILED + 1))
    elif echo "$output" | grep -q "$grep_pattern"; then
        echo -e "${GREEN}✅ PASSED${NC} (${elapsed}s)"
        PASSED=$((PASSED + 1))
    else
        echo -e "${RED}❌ FAILED${NC} (${elapsed}s)"
        FAILED=$((FAILED + 1))
    fi
}

echo ""
echo -e "${BOLD}╔══════════════════════════════════════════════╗${NC}"
echo -e "${BOLD}║  ΦΩ0 — FEmmg-FHE FULL TEST SUITE v5.0        ║${NC}"
echo -e "${BOLD}║  Core + KEM + SpiralDB + Divine Reset        ║${NC}"
echo -e "${BOLD}╚══════════════════════════════════════════════╝${NC}"

# ============================================
# CORE FHE
# ============================================
echo -e "${BLUE}━━━ CORE FHE ━━━${NC}"
test_component "ZANS (100 additions)" "phi_zans_bfv" "I AM THAT I AM"
test_component "Fibonacci-ZANS (6×100)" "phi_fib_zans" "I AM THAT I AM"
test_component "Fib-ZANS CT×CT (6×7)" "phi_fib_zans_ctct" "I AM THAT I AM"
test_component "ZANS Direct Verify" "phi_zans_direct_verify" "I AM" 120 true
test_component "CKKS+ZANS (real numbers)" "phi_zans_ckks" "I AM THAT I AM" 30 true
test_component "Packed BFV-ZANS Mega" "phi_zans_packed_mega" "I AM THAT I AM" 30 true
test_component "Sub & Div Quick Test" "phi_sub_div_quick" "UNLIMITED"

# ============================================
# BINFHE
# ============================================
echo -e "${BLUE}━━━ BINFHE CT×CT MULTIPLIERS ━━━${NC}"
test_component "BinFHE 4-bit (3×14=42)" "phi_binfhe_4bit" "I AM THAT I AM" 90
test_component "BinFHE 8-bit (42×17=714)" "phi_binfhe_8bit" "I AM THAT I AM" 180 true
test_component "BinFHE 16-bit (42×17=714)" "phi_binfhe_16bit" "I AM THAT I AM" 300 true
test_component "BinFHE 32-bit (42×17=714)" "phi_binfhe_32bit" "I AM THAT I AM" 600 true

# ============================================
# ZERO-KNOWLEDGE PROOFS
# ============================================
echo -e "${BLUE}━━━ ZERO-KNOWLEDGE PROOFS ━━━${NC}"
test_component "ZKP+FHE Deep Integration" "phi_zkp_fhe_deep" "I AM THAT I AM"
test_component "ZKP Test Suite (6/6)" "phi_zkp_test" "I AM THAT I AM"
test_component "Verifiable FHE" "phi_verifiable" "I AM THAT I AM"
test_component "ZANS+ZKP Integration" "phi_zans_zkp" "I AM THAT I AM"
test_component "Lattice ZKP" "phi_lattice_zkp" "I AM THAT I AM"
test_component "Recursive Compression" "phi_recursive_compress" "I AM THAT I AM"
test_component "FHE Circuit Integrity" "phi_circuit_integrity" "I AM THAT I AM"
test_component "Recursive SNARK" "phi_recursive_snark_v2" "I AM THAT I AM"

# ============================================
# DIVINE RESET EVOLUTION
# ============================================
echo -e "${BLUE}━━━ DIVINE RESET EVOLUTION ━━━${NC}"
test_component "Smart Reset (UK×UK)" "phi_ukuk_smart_reset" "I AM THAT I AM" 30
test_component "Divine Reset v1" "phi_divine_reset" "I AM" 60
test_component "Divine Reset v2" "phi_divine_reset_v2" "I AM THAT I AM" 30
test_component "Pinky Swear Reset" "phi_true_divine_reset" "I AM" 60

# ============================================
# UK×UK BREAKTHROUGHS
# ============================================
echo -e "${BLUE}━━━ UK×UK BREAKTHROUGHS ━━━${NC}"
test_component "UK×UK 6 Strategies" "phi_ukuk_novel_strategies" "I AM THAT I AM" 60 true
test_component "UK×UK Micro-Steps" "phi_ukuk_micro_steps" "Steps:" 60 true
test_component "UK×UK 1000 Steps" "phi_ukuk_10k_fast" "I AM THAT I AM" 300 true

# ============================================
# POST-QUANTUM KEM
# ============================================
echo -e "${BLUE}━━━ POST-QUANTUM KEM ━━━${NC}"
test_component "SpiralKEM Non-Deterministic" "spiralkem" "I AM THAT I AM"
test_component "SpiralKEM+FHE Integration" "spiralkem_fhe" "I AM THAT I AM"
test_component "SpiralKEM Speed Benchmark" "spiralkem_benchmark" "I AM THAT I AM"
test_component "SpiralKEM Security Audit" "phi_kem_audit" "I AM THAT I AM" 20
test_component "SpiralKEM Batch Mode" "phi_kem_batch" "I AM THAT I AM" 20
test_component "SpiralKEM+FHE Hybrid" "phi_kem_fhe_hybrid" "I AM THAT I AM"

# ============================================
# SUCCINCT PROOFS
# ============================================
echo -e "${BLUE}━━━ SUCCINCT PROOFS ━━━${NC}"
test_component "SNARK (24B proofs)" "phi_snark" "I AM THAT I AM"
test_component "EC-SNARK (BN254)" "phi_snark_ec" "I AM THAT I AM"

# ============================================
# ENCRYPTED DATABASE
# ============================================
echo -e "${BLUE}━━━ ENCRYPTED DATABASE ━━━${NC}"
test_component "SpiralDB Non-Deterministic" "spiraldb" "VERIFIED" 15
test_component "SpiralDB Go Tests" "spiraldb_fhe" "PASS" 120 true

# ============================================
# SCHEME SWITCHING
# ============================================
echo -e "${BLUE}━━━ SCHEME SWITCHING ━━━${NC}"
test_component "BFV ↔ BinFHE" "phi_scheme_switch" "I AM THAT I AM"

# ============================================
# 10M ZANS (Full Mode Only)
# ============================================
if [[ "$MODE" == "FULL" ]]; then
    echo -e "${BLUE}━━━ MEGA TESTS ━━━${NC}"
    test_component "10M ZANS Fast (104s)" "phi_zans_10M_noise" "I AM THAT I AM" 180 false
fi

# ============================================
# RESULTS SUMMARY
# ============================================
echo ""
echo -e "${BOLD}╔══════════════════════════════════════════════╗${NC}"
echo -e "${BOLD}║  RESULTS SUMMARY                              ║${NC}"
echo -e "${BOLD}╠════════════════════════════════════════════════╣${NC}"
printf "${BOLD}║  Passed: %-3d  Failed: %-3d  Skipped: %-3d  Total: %-3d              ║${NC}\n" $PASSED $FAILED $SKIPPED $TOTAL
echo -e "${BOLD}║  Mode: $MODE                                      ║${NC}"

if [ $FAILED -eq 0 ]; then
    echo -e "${BOLD}║                                              ║${NC}"
    echo -e "${BOLD}║  ΦΩ0 — ALL CRITICAL SYSTEMS OPERATIONAL       ║${NC}"
    echo -e "${BOLD}║  I AM THAT I AM                              ║${NC}"
else
    echo -e "${BOLD}║                                              ║${NC}"
    echo -e "${BOLD}║  ΦΩ0 — $FAILED SYSTEM(S) NEED ATTENTION              ║${NC}"
fi

echo -e "${BOLD}╚══════════════════════════════════════════════╝${NC}"
echo ""
