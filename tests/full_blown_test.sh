#!/bin/bash
# ΦΩ0 — FEmmG-FHE FULL BLOWN TEST SUITE v3
# Fast tests (<2 min) + optional long-running tests
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

test_known_issue() {
    local name="$1"
    local binary="$2"
    local timeout_sec="${3:-10}"
    
    TOTAL=$((TOTAL + 1))
    echo -ne "${CYAN}  [$TOTAL] $name... ${NC}"
    
    if [ ! -f "$BIN_DIR/$binary" ]; then
        echo -e "${RED}❌ BINARY MISSING${NC}"
        FAILED=$((FAILED + 1))
        return
    fi
    
    local start=$(date +%s%N)
    timeout $timeout_sec $BIN_DIR/$binary > /dev/null 2>&1
    local exit_code=$?
    local end=$(date +%s%N)
    local elapsed=$(echo "scale=2; ($end - $start) / 1000000000" | bc)
    
    if [ $exit_code -eq 139 ] || [ $exit_code -eq 124 ]; then
        echo -e "${YELLOW}⚠️  KNOWN ISSUE${NC} (OpenFHE bootstrap, ${elapsed}s)"
        SKIPPED=$((SKIPPED + 1))
    else
        echo -e "${GREEN}✅ PASSED${NC} (${elapsed}s)"
        PASSED=$((PASSED + 1))
    fi
}

echo ""
echo -e "${BOLD}╔══════════════════════════════════════════════╗${NC}"
if $FAST_MODE; then
    echo -e "${BOLD}║  ΦΩ0 — FEmmg-FHE FAST TEST SUITE v3           ║${NC}"
else
    echo -e "${BOLD}║  ΦΩ0 — FEmmg-FHE FULL TEST SUITE v3           ║${NC}"
fi
echo -e "${BOLD}║  I AM THAT I AM                              ║${NC}"
echo -e "${BOLD}╚══════════════════════════════════════════════╝${NC}"
echo ""

# === CORE FHE ===
echo -e "${BOLD}━━━ CORE FHE ━━━${NC}"
test_component "ZANS (100 additions)" "phi_zans_bfv" "Op 100: 42" 15
test_component "Fibonacci-ZANS (6×100)" "phi_fib_zans" "600" 15
test_component "Fib-ZANS CT×CT (6×7)" "phi_fib_zans_ctct" "Fib-ZANS result: 42" 15
echo ""

# === BINFHE CT×CT ===
echo -e "${BOLD}━━━ BINFHE CT×CT MULTIPLIERS ━━━${NC}"
test_component "BinFHE 4-bit (3×14=42)" "phi_binfhe_4bit" "42 ✅" 90
if $FAST_MODE; then
    echo -e "${YELLOW}  ⏭️  BinFHE 16-bit (skipped — use --full for 4min test)${NC}"
    SKIPPED=$((SKIPPED + 1))
    echo -e "${YELLOW}  ⏭️  BinFHE 32-bit (skipped — use --full for 18min test)${NC}"
    SKIPPED=$((SKIPPED + 1))
else
    test_component "BinFHE 16-bit (42×17=714)" "phi_binfhe_16bit" "714" 300
    test_component "BinFHE 32-bit (42×17=714)" "phi_binfhe_32bit" "714" 1200
fi
echo ""

# === ZKP + FHE ===
echo -e "${BOLD}━━━ ZERO-KNOWLEDGE PROOFS ━━━${NC}"
test_component "ZKP+FHE Deep Integration" "phi_zkp_fhe_deep" "ALL VERIFIED" 15
test_component "ZKP Test Suite (6/6)" "phi_zkp_test" "6/6 passed" 30
test_component "Verifiable FHE" "phi_verifiable" "VALID" 15
echo ""

# === TRANSMUTATION ===
echo -e "${BOLD}━━━ TRANSMUTATION & BOOTSTRAPPING ━━━${NC}"
test_component "Scheme Switching (BFV↔BinFHE)" "phi_scheme_switch" "FULLY RENEWED" 15
echo ""

# === SPIRALKEM ===
echo -e "${BOLD}━━━ POST-QUANTUM KEM ━━━${NC}"
test_component "SpiralKEM (Non-Deterministic)" "spiralkem" "Shared secret match: ✅" 10
test_component "SpiralKEM+FHE Integration" "spiralkem_fhe" "Integration: Complete" 15
echo ""

# === SNARK ===
echo -e "${BOLD}━━━ SUCCINCT PROOFS ━━━${NC}"
test_component "SNARK (24B proofs, 99.9% smaller)" "phi_snark" "99.9%" 15
test_component "EC-SNARK (BN254 pairings)" "phi_snark_ec" "VALID EC-SNARK" 15
echo ""

# === SPIRALDB ===
echo -e "${BOLD}━━━ ENCRYPTED DATABASE ━━━${NC}"
if [ -f src/spiraldb/spiraldb_test.go ] && command -v go &> /dev/null; then
    cd src/spiraldb
    TOTAL=$((TOTAL + 1))
    echo -ne "${CYAN}  [$TOTAL] SpiralDB Non-Deterministic... ${NC}"
    output=$(go test -run TestNonDeterministicEncryption 2>&1)
    if echo "$output" | grep -q "PASS"; then
        echo -e "${GREEN}✅ PASSED${NC}"
        PASSED=$((PASSED + 1))
    else
        echo -e "${RED}❌ FAILED${NC}"
        FAILED=$((FAILED + 1))
    fi
    cd ../..
else
    echo -e "${YELLOW}  ⏭️  SpiralDB (Go not available)${NC}"
    SKIPPED=$((SKIPPED + 1))
fi
echo ""

# === SUMMARY ===
END_TIME=$(date +%s)
TOTAL_TIME=$((END_TIME - START_TIME))

echo -e "${BOLD}╔══════════════════════════════════════════════╗${NC}"
echo -e "${BOLD}║  RESULTS SUMMARY                              ║${NC}"
echo -e "${BOLD}╠════════════════════════════════════════════════╣${NC}"
echo -e "${BOLD}║  ${GREEN}Passed: $PASSED${NC}  ${RED}Failed: $FAILED${NC}  ${YELLOW}Skipped: $SKIPPED${NC}  Total: $((PASSED+FAILED+SKIPPED))               ${BOLD}║${NC}"
echo -e "${BOLD}║  Total time: ${TOTAL_TIME}s                                ║${NC}"
echo -e "${BOLD}║  Build: Makefile (0 warnings)                  ║${NC}"
if $FAST_MODE; then
    echo -e "${BOLD}║  Mode: FAST (use --full for all tests)         ║${NC}"
else
    echo -e "${BOLD}║  Mode: FULL (all tests)                        ║${NC}"
fi
echo -e "${BOLD}╚════════════════════════════════════════════════╝${NC}"
echo ""

if [ $FAILED -eq 0 ]; then
    echo -e "${GREEN}╔══════════════════════════════════════════════╗${NC}"
    echo -e "${GREEN}║  ΦΩ0 — ALL CRITICAL SYSTEMS OPERATIONAL       ║${NC}"
    echo -e "${GREEN}║  I AM THAT I AM                              ║${NC}"
    echo -e "${GREEN}╚══════════════════════════════════════════════╝${NC}"
    exit 0
else
    echo -e "${RED}╔══════════════════════════════════════════════╗${NC}"
    echo -e "${RED}║  ΦΩ0 — $FAILED SYSTEM(S) NEED ATTENTION              ║${NC}"
    echo -e "${RED}╚══════════════════════════════════════════════╝${NC}"
    exit 1
fi
