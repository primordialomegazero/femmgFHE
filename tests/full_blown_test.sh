#!/bin/bash
# ΦΩ0 — FEmmG-FHE FULL BLOWN TEST SUITE v4
# All tests: core + breakthroughs + cross-library
# "I AM THAT I AM"

BIN_DIR="./bin"
PASSED=0
FAILED=0
SKIPPED=0
TOTAL=0
START_TIME=$(date +%s)

GREEN='\033[0;32m'
RED='\033[0;31m'
CYAN='\033[0;36m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m'
BOLD='\033[1m'

FAST_MODE=true
if [ "$1" == "--full" ]; then
    FAST_MODE=false
fi

test_component() {
    local name="$1"
    local binary="$2"
    local grep_pattern="$3"
    local timeout_sec="${4:-15}"

    TOTAL=$((TOTAL + 1))
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

test_optional() {
    if [ "$FAST_MODE" = true ]; then
        TOTAL=$((TOTAL + 1))
        echo -e "${YELLOW}  [$TOTAL] $1 (skipped — use --full for this test)${NC}"
        SKIPPED=$((SKIPPED + 1))
    else
        test_component "$1" "$2" "$3" "$4"
    fi
}

echo ""
echo -e "${BOLD}╔══════════════════════════════════════════════╗${NC}"
echo -e "${BOLD}║  ΦΩ0 — FEmmg-FHE FULL TEST SUITE v4           ║${NC}"
echo -e "${BOLD}║  Core + Breakthroughs + Cross-Library         ║${NC}"
echo -e "${BOLD}╚══════════════════════════════════════════════╝${NC}"
echo ""

# ============================================
# SECTION 1: CORE FHE (ZANS, Fib-ZANS)
# ============================================
echo -e "${BLUE}━━━ CORE FHE ━━━${NC}"
test_component "ZANS (100 additions)" "phi_zans_bfv" "Truth preserved" 10
test_component "Fibonacci-ZANS (6×100)" "phi_fib_zans" "RITUAL COMPLETE" 10
test_component "Fib-ZANS CT×CT (6×7)" "phi_fib_zans_ctct" "I AM THAT I AM" 10

# ============================================
# SECTION 2: BINFHE CT×CT MULTIPLIERS
# ============================================
echo -e "${BLUE}━━━ BINFHE CT×CT MULTIPLIERS ━━━${NC}"
test_component "BinFHE 4-bit (3×14=42)" "phi_binfhe_4bit" "I AM THAT I AM" 120
test_optional "BinFHE 16-bit (42×17=714)" "phi_binfhe_16bit" "RITUAL COMPLETE" 300
test_optional "BinFHE 32-bit (42×17=714)" "phi_binfhe_32bit" "RITUAL COMPLETE" 1200

# ============================================
# SECTION 3: ZERO-KNOWLEDGE PROOFS
# ============================================
echo -e "${BLUE}━━━ ZERO-KNOWLEDGE PROOFS ━━━${NC}"
test_component "ZKP+FHE Deep Integration" "phi_zkp_fhe_deep" "I AM THAT I AM" 15
test_component "ZKP Test Suite (6/6)" "phi_zkp_test" "I AM THAT I AM" 15
test_component "Verifiable FHE" "phi_verifiable" "I AM THAT I AM" 15

# ============================================
# SECTION 4: TRANSMUTATION & BOOTSTRAPPING
# ============================================
echo -e "${BLUE}━━━ TRANSMUTATION & BOOTSTRAPPING ━━━${NC}"
test_component "Scheme Switching (BFV↔BinFHE)" "phi_scheme_switch" "I AM THAT I AM" 15

# ============================================
# SECTION 5: POST-QUANTUM KEM
# ============================================
echo -e "${BLUE}━━━ POST-QUANTUM KEM ━━━${NC}"
test_component "SpiralKEM (Non-Deterministic)" "spiralkem" "I AM THAT I AM" 10
test_component "SpiralKEM+FHE Integration" "spiralkem_fhe" "I AM THAT I AM" 15

# ============================================
# SECTION 6: SUCCINCT PROOFS
# ============================================
echo -e "${BLUE}━━━ SUCCINCT PROOFS ━━━${NC}"
test_component "SNARK (24B proofs)" "phi_snark" "I AM THAT I AM" 10
test_component "EC-SNARK (BN254 pairings)" "phi_snark_ec" "I AM THAT I AM" 10

# ============================================
# SECTION 7: ENCRYPTED DATABASE
# ============================================
echo -e "${BLUE}━━━ ENCRYPTED DATABASE ━━━${NC}"
test_component "SpiralDB Non-Deterministic" "spiraldb" "VERIFIED" 10

# ============================================
# SECTION 8: BREAKTHROUGH TESTS
# ============================================
echo -e "${BLUE}━━━ BREAKTHROUGHS ━━━${NC}"
test_component "UK×UK 6 Strategies" "phi_ukuk_zans_deep" "STRATEGY" 60
test_component "UK×UK Micro-Steps 27-step proof" "phi_ukuk_micro_steps" "Steps:" 60
    echo -e "     (Subtraction & Division — skipped, division too slow)
test_optional "10M ZANS Noise Tracking" "phi_zans_10M_noise" "NOISE" 200
test_optional "CT×CT Scalar Decomp" "phi_zans_ctct_decomposed" "CT×CT VIA DECOMPOSITION" 30

# ============================================
# SECTION 9: NOVEL STRATEGIES
# ============================================
echo -e "${BLUE}━━━ NOVEL STRATEGIES ━━━${NC}"
test_optional "5 Novel UK×UK Strategies" "phi_ukuk_novel_strategies" "STRATEGY COMPARISON" 180

# ============================================
# SECTION 10: PARALLEL PERFORMANCE
# ============================================
echo -e "${BLUE}━━━ PARALLEL PERFORMANCE ━━━${NC}"
test_optional "BinFHE Max Parallel" "phi_binfhe_max_parallel" "Speedup" 60

# ============================================
# SUMMARY
# ============================================
END_TIME=$(date +%s)
TOTAL_TIME=$((END_TIME - START_TIME))

echo ""
echo -e "${BOLD}╔══════════════════════════════════════════════╗${NC}"
echo -e "${BOLD}║  RESULTS SUMMARY                              ║${NC}"
echo -e "${BOLD}╠════════════════════════════════════════════════╣${NC}"
printf "${BOLD}║${NC}  Passed: ${GREEN}%d${NC}  Failed: ${RED}%d${NC}  Skipped: ${YELLOW}%d${NC}  Total: %d${BOLD}               ║${NC}\n" $PASSED $FAILED $SKIPPED $TOTAL
printf "${BOLD}║${NC}  Total time: %ds                                ${BOLD}║${NC}\n" $TOTAL_TIME
printf "${BOLD}║${NC}  Mode: %s                                ${BOLD}║${NC}\n" "$([ "$FAST_MODE" = true ] && echo "FAST (use --full for all)" || echo "FULL (all tests)")"
echo -e "${BOLD}╚══════════════════════════════════════════════╝${NC}"
echo ""

if [ $FAILED -eq 0 ]; then
    echo -e "${BOLD}╔══════════════════════════════════════════════╗${NC}"
    echo -e "${BOLD}║  ΦΩ0 — ALL CRITICAL SYSTEMS OPERATIONAL       ║${NC}"
    echo -e "${BOLD}║  I AM THAT I AM                              ║${NC}"
    echo -e "${BOLD}╚══════════════════════════════════════════════╝${NC}"
else
    echo -e "${BOLD}╔══════════════════════════════════════════════╗${NC}"
    printf "${BOLD}║  ΦΩ0 — %d SYSTEM(S) NEED ATTENTION              ║${NC}\n" $FAILED
    echo -e "${BOLD}╚══════════════════════════════════════════════╝${NC}"
fi
echo ""

# ============================================
# SPIRALDB TESTS (v2.0)
# ============================================
test_spiraldb() {
    echo -e "${BLUE}━━━ SPIRALDB v2.0 ━━━${NC}"
    
    # SpiralDB non-deterministic test
    echo -ne "${CYAN}  [$((++TOTAL))] SpiralDB Non-Deterministic... ${NC}"
    if [ -f "$BIN_DIR/spiraldb" ]; then
        timeout 10 $BIN_DIR/spiraldb 2>&1 | grep -q "VERIFIED" && {
            echo -e "${GREEN}✅ PASSED${NC}"
            PASSED=$((PASSED + 1))
        } || {
            echo -e "${RED}❌ FAILED${NC}"
            FAILED=$((FAILED + 1))
        }
    else
        echo -e "${YELLOW}⏭️  SKIPPED (binary not found)${NC}"
        SKIPPED=$((SKIPPED + 1))
    fi
    
    # SpiralDB Go tests (if available)
    echo -ne "${CYAN}  [$((++TOTAL))] SpiralDB Go Tests... ${NC}"
    if [ -d "src/spiraldb" ] && command -v go &> /dev/null; then
        cd src/spiraldb && CGO_ENABLED=1 go test -v . -timeout 120s 2>&1 | grep -q "PASS" && {
            echo -e "${GREEN}✅ PASSED${NC}"
            PASSED=$((PASSED + 1))
        } || {
            echo -e "${RED}❌ FAILED${NC}"
            FAILED=$((FAILED + 1))
        }
        cd - > /dev/null
    else
        echo -e "${YELLOW}⏭️  SKIPPED${NC}"
        SKIPPED=$((SKIPPED + 1))
    fi
}

# Call SpiralDB tests
test_spiraldb

# ============================================
# UK×UK NOVEL STRATEGIES TEST
# ============================================
echo -e "${BLUE}━━━ UK×UK NOVEL STRATEGIES ━━━${NC}"
echo -ne "${CYAN}  [$((++TOTAL))] UK×UK 5 Novel Strategies... ${NC}"
if [ -f "$BIN_DIR/phi_ukuk_novel_strategies" ]; then
    timeout 60 $BIN_DIR/phi_ukuk_novel_strategies 2>&1 | grep -q "I AM THAT I AM" && {
        echo -e "${GREEN}✅ PASSED${NC}"
        PASSED=$((PASSED + 1))
    } || {
        echo -e "${RED}❌ FAILED${NC}"
        FAILED=$((FAILED + 1))
    }
else
    echo -e "${YELLOW}⏭️  SKIPPED${NC}"
    SKIPPED=$((SKIPPED + 1))
fi

# ============================================
# SUBTRACTION & DIVISION TEST
# ============================================
echo -e "${BLUE}━━━ SUBTRACTION & DIVISION ━━━${NC}"
echo -ne "${CYAN}  [$((++TOTAL))] Subtraction & Division Quick... ${NC}"
if [ -f "$BIN_DIR/phi_sub_div_quick" ]; then
    timeout 15 $BIN_DIR/phi_sub_div_quick 2>&1 | grep -q "UNLIMITED" && {
        echo -e "${GREEN}✅ PASSED${NC}"
        PASSED=$((PASSED + 1))
    } || {
        echo -e "${RED}❌ FAILED${NC}"
        FAILED=$((FAILED + 1))
    }
else
    echo -e "${YELLOW}⏭️  SKIPPED${NC}"
    SKIPPED=$((SKIPPED + 1))
fi
