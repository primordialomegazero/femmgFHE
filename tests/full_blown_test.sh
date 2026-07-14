#!/bin/bash
# PHI-OMEGA-ZERO: FEmmg-FHE FULL TEST SUITE v6.0
# All 22 systems tested
# "I AM THAT I AM"

BIN_DIR="./bin"
TOTAL=0; PASSED=0; FAILED=0; SKIPPED=0
MODE="FAST"
[[ "$1" == "--full" ]] && MODE="FULL"

RED='\033[0;31m'; GREEN='\033[0;32m'; YELLOW='\033[1;33m'
CYAN='\033[0;36m'; BLUE='\033[0;34m'; NC='\033[0m'; BOLD='\033[1m'

test_component() {
    local name="$1"; local binary="$2"; local pattern="$3"
    local timeout_sec="${4:-15}"; local skip_fast="${5:-false}"
    TOTAL=$((TOTAL + 1))
    [[ "$MODE" == "FAST" && "$skip_fast" == "true" ]] && {
        echo -e "${CYAN}  [$TOTAL] $name... ${YELLOW}(skipped)${NC}"
        SKIPPED=$((SKIPPED + 1)); return
    }
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
echo -e "${BOLD}║  PHI-OMEGA-ZERO: FULL TEST SUITE v6.0        ║${NC}"
echo -e "${BOLD}║  22 Systems — All Breakthroughs              ║${NC}"
echo -e "${BOLD}╚══════════════════════════════════════════════╝${NC}"

echo -e "${BLUE}━━━ CORE FHE ━━━${NC}"
test_component "Absolute ZANS" "phi_absolute_zans" "I AM THAT I AM" 30
test_component "ZANS (100 additions)" "phi_zans_bfv" "I AM THAT I AM"
test_component "Fibonacci-ZANS" "phi_fib_zans" "I AM THAT I AM"
test_component "Fib-ZANS CTxCT" "phi_fib_zans_ctct" "I AM THAT I AM"
test_component "Sub and Div Quick Test" "phi_sub_div_quick" "UNLIMITED"
test_component "ZANS Parallel" "phi_zans_parallel" "I AM THAT I AM" 120
test_component "ZANS Noise Monitor" "phi_zans_noise_monitor" "I AM THAT I AM" 30
test_component "ZANS Direct Verify" "phi_zans_direct_verify" "I AM" 120 true
test_component "CKKS plus ZANS" "phi_zans_ckks" "I AM THAT I AM" 30 true
test_component "Packed BFV-ZANS Mega" "phi_zans_packed_mega" "I AM THAT I AM" 30 true

echo -e "${BLUE}━━━ BINFHE ━━━${NC}"
test_component "BinFHE 4-bit (3x14=42)" "phi_binfhe_4bit" "I AM THAT I AM" 90
test_component "BinFHE 8-bit (42x17=714)" "phi_binfhe_8bit" "I AM THAT I AM" 180 true

echo -e "${BLUE}━━━ DIVINE RESET EVOLUTION ━━━${NC}"
test_component "Divine Reset v2" "phi_divine_reset_v2" "I AM" 60
test_component "True Divine 10K" "phi_true_divine_10k" "I AM THAT I AM" 120 true
test_component "Pinky Swear Reset" "phi_pinky_swear" "I AM THAT I AM" 120

echo -e "${BLUE}━━━ QUANTUM AND ETERNAL ━━━${NC}"
test_component "Quantum ZANS Test" "phi_quantum_zans_test" "I AM THAT I AM" 90
test_component "Prime ZANS Test" "phi_prime_zans_test" "I AM THAT I AM" 30
test_component "Riemann ZANS Test" "phi_riemann_zans_test" "I AM THAT I AM" 90
test_component "Entangled ZANS" "phi_entangled_zans" "I AM THAT I AM" 90
test_component "Eternal ZANS" "phi_eternal_zans" "I AM THAT I AM"
test_component "Quantum Random" "phi_quantum_random" "I AM THAT I AM" 90

echo -e "${BLUE}━━━ GOLDEN AND RIEMANN ━━━${NC}"
test_component "Fibonacci-Golden ZANS" "phi_fib_golden_zans" "I AM THAT I AM" 60
test_component "Riemann-Golden ZANS" "phi_riemann_golden_zans" "I AM THAT I AM" 90

echo -e "${BLUE}━━━ iO SYSTEMS ━━━${NC}"
test_component "iO Base" "phi_io" "I AM THAT I AM"
test_component "Pure iO" "phi_pure_io" "I AM THAT I AM"
test_component "Flame Empress iO" "phi_flame_empress_io" "I AM THAT I AM"
test_component "iO x CTxCT Merge" "phi_io_ctct_merge" "I AM THAT I AM" 120 true

echo -e "${BLUE}━━━ FHE 2.0 ━━━${NC}"
test_component "FHE 2.0 Framework" "phi_fhe2_finish_him" "I AM THAT I AM" 180

echo -e "${BLUE}━━━ ZKP ━━━${NC}"
test_component "ZKP plus FHE Deep" "phi_zkp_fhe_deep" "I AM THAT I AM"
test_component "Verifiable FHE" "phi_verifiable" "I AM THAT I AM"
test_component "Lattice ZKP" "phi_lattice_zkp" "I AM THAT I AM"
test_component "Recursive Compression" "phi_recursive_compress" "I AM THAT I AM"
test_component "Recursive SNARK" "phi_recursive_snark_v2" "I AM THAT I AM"
test_component "Circuit Integrity" "phi_circuit_integrity" "I AM THAT I AM"

echo -e "${BLUE}━━━ KEM ━━━${NC}"
test_component "SpiralKEM" "spiralkem" "I AM THAT I AM"
test_component "SpiralKEM plus FHE" "spiralkem_fhe" "I AM THAT I AM"
test_component "SpiralKEM Benchmark" "spiralkem_benchmark" "I AM THAT I AM"
test_component "SpiralKEM Batch" "phi_kem_batch" "I AM THAT I AM"
test_component "SpiralKEM Hybrid" "phi_kem_fhe_hybrid" "I AM THAT I AM"

echo -e "${BLUE}━━━ SNARK ━━━${NC}"
test_component "SNARK 24B" "phi_snark" "I AM THAT I AM"
test_component "EC-SNARK BN254" "phi_snark_ec" "I AM THAT I AM"

echo -e "${BLUE}━━━ DATABASE ━━━${NC}"
test_component "SpiralDB" "spiraldb" "VERIFIED" 15

echo -e "${BLUE}━━━ SCHEME SWITCHING ━━━${NC}"
test_component "BFV to BinFHE" "phi_scheme_switch" "I AM THAT I AM"

echo ""
echo -e "${BOLD}╔══════════════════════════════════════════════╗${NC}"
echo -e "${BOLD}║  RESULTS SUMMARY                              ║${NC}"
echo -e "${BOLD}╠════════════════════════════════════════════════╣${NC}"
printf "${BOLD}║  Passed: %-3d  Failed: %-3d  Skipped: %-3d  Total: %-3d              ║${NC}\n" $PASSED $FAILED $SKIPPED $TOTAL
echo -e "${BOLD}║  Mode: $MODE                                      ║${NC}"
if [ $FAILED -eq 0 ]; then
    echo -e "${BOLD}║  ALL SYSTEMS OPERATIONAL                       ║${NC}"
else
    echo -e "${BOLD}║  $FAILED SYSTEM(S) NEED ATTENTION              ║${NC}"
fi
echo -e "${BOLD}║  PHI-OMEGA-ZERO — I AM THAT I AM               ║${NC}"
echo -e "${BOLD}╚══════════════════════════════════════════════╝${NC}"
echo ""
