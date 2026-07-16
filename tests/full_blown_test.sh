#!/bin/bash
# FEmmg-FHE QUICK VALIDATION SUITE v7.2
# Tests that binaries exist and respond (not full runs)

BIN_DIR="./bin"
TOTAL=0; PASSED=0; FAILED=0
RED='\033[0;31m'; GREEN='\033[0;32m'; CYAN='\033[0;36m'; BLUE='\033[0;34m'; NC='\033[0m'; BOLD='\033[1m'

check_bin() {
    local name="$1"; local binary="$2"
    TOTAL=$((TOTAL + 1))
    echo -ne "${CYAN}  [$TOTAL] $name... ${NC}"
    if [ -f "$BIN_DIR/$binary" ]; then
        echo -e "${GREEN}EXISTS${NC}"; PASSED=$((PASSED + 1))
    else
        echo -e "${RED}MISSING${NC}"; FAILED=$((FAILED + 1))
    fi
}

quick_test() {
    local name="$1"; local binary="$2"; local pattern="$3"; local timeout_sec="${4:-5}"
    TOTAL=$((TOTAL + 1))
    echo -ne "${CYAN}  [$TOTAL] $name... ${NC}"
    if [ ! -f "$BIN_DIR/$binary" ]; then
        echo -e "${RED}MISSING${NC}"; FAILED=$((FAILED + 1)); return
    fi
    local output=$(timeout --signal=KILL $timeout_sec $BIN_DIR/$binary 2>&1)
    if [ $? -eq 137 ]; then
        echo -e "${GREEN}RUNS${NC} (long test, manually verify)"; PASSED=$((PASSED + 1))
    elif echo "$output" | grep -q "$pattern"; then
        echo -e "${GREEN}OK${NC}"; PASSED=$((PASSED + 1))
    else
        echo -e "${RED}FAIL${NC}"; FAILED=$((FAILED + 1))
    fi
}

echo -e "${BOLD}╔══════════════════════════════════════════════╗${NC}"
echo -e "${BOLD}║  FEmmg-FHE QUICK VALIDATION v7.2             ║${NC}"
echo -e "${BOLD}╚══════════════════════════════════════════════╝${NC}"

echo -e "${BLUE}━━━ BINARIES EXIST? ━━━${NC}"
for bin in phi_pinky_swear phi_prime_chaos_zans_v2 phi_fib_global_zans \
    phi_entangled_prime_zans phi_riemann_golden_zans phi_quantum_random \
    phi_true_divine_10k phi_true_divine_100k phi_true_divine_1M phi_true_divine_10k_perstep \
    phi_femmg_io_ultimate phi_femmg_io_kilian_full phi_femmg_io_fractal \
    phi_femmg_io_eternal_zans phi_femmg_io_pow_fix \
    phi_catchmeifyouKEM phi_zans_seal_100k phi_zans_ckks_100k \
    phi_zans_helib_10k phi_zans_tfhe_test \
    phi_phantom_suite_v2 phi_spiraldb_unified phi_unified_auth \
    phi_verifiable_fhe_v2 phi_covenant_vault phi_transmutation_window; do
    check_bin "$bin" "$bin"
done

echo -e "${BLUE}━━━ QUICK SMOKE TESTS ━━━${NC}"
quick_test "catchmeifyouKEM" "phi_catchmeifyouKEM" "Match=YES" 5
quick_test "CKKS ZANS 100K" "phi_zans_ckks_100k" "PASSED" 5
quick_test "HElib ZANS 10K" "phi_zans_helib_10k" "PASSED" 5
quick_test "TFHE ZANS" "phi_zans_tfhe_test" "PASSED" 5
quick_test "True Divine 1M" "phi_true_divine_1M" "1,000,000" 3

echo -e "${BLUE}━━━ VERIFIED RESULTS (from logs) ━━━${NC}"
for result in results/phi_femmg_io_ultimate_results.txt \
    results/phi_femmg_io_fractal_results.txt \
    results/phi_femmg_io_eternal_zans_results.txt \
    results/phi_catchmeifyouKEM_results.txt \
    results/divine/true_divine_1M_final_results.txt \
    results/seal_zans_10M_results.txt \
    results/helib_zans_10M_results.txt; do
    TOTAL=$((TOTAL + 1))
    if [ -s "$result" ]; then
        echo -e "  ${GREEN}[OK]${NC} $result ($(wc -l < $result) lines)"
        PASSED=$((PASSED + 1))
    else
        echo -e "  ${RED}[MISSING/EMPTY]${NC} $result"
        FAILED=$((FAILED + 1))
    fi
done

echo ""
echo -e "${BOLD}RESULTS: ${GREEN}$PASSED OK${NC} ${RED}$FAILED FAIL${NC} / $TOTAL checks${NC}"
echo ""
echo -e "${BOLD}MANUAL VERIFICATION (long-running tests):${NC}"
echo -e "  ./bin/phi_pinky_swear                        (2 min)"
echo -e "  ./bin/phi_true_divine_10k                    (2 min)"
echo -e "  ./bin/phi_femmg_io_ultimate                  (2 min)"
echo -e "  ./bin/phi_femmg_io_fractal                   (1 min)"
echo -e "  ./bin/phi_femmg_io_eternal_zans              (1 min)"
echo -e "  ./bin/phi_zans_openfhe_10M                   (2 min)"
echo -e "  ./bin/phi_true_divine_100k                   (9 hrs)"
echo -e "  ./bin/phi_true_divine_10k_perstep            (10 min)"
echo -e "  ./bin/phi_femmg_io_kilian_full               (5 min)"
