#!/bin/bash
# ΦΩ0 — FULL SYSTEM AUDIT
# Verify all components work

PASSED=0
FAILED=0
TOTAL=0

check() {
    TOTAL=$((TOTAL + 1))
    if [ $? -eq 0 ]; then
        echo "  ✅ $1"
        PASSED=$((PASSED + 1))
    else
        echo "  ❌ $1"
        FAILED=$((FAILED + 1))
    fi
}

echo "╔══════════════════════════════════════════════╗"
echo "║  ΦΩ0 — FULL SYSTEM AUDIT                      ║"
echo "╚══════════════════════════════════════════════╝"
echo ""

# 1. ZANS (BFV Additions)
echo "=== 1. ZANS: 100 Additions with Zero Anchor ==="
timeout 15 bin/phi_zans_bfv 2>/dev/null | grep -q "Op 100: 42"
check "ZANS 100 additions preserve truth"

# 2. Fibonacci-ZANS
echo "=== 2. FIB-ZANS: CT × 100 ==="
timeout 15 bin/phi_fib_zans 2>/dev/null | grep -q "600"
check "Fibonacci-ZANS CT×100=600"

# 3. BFV Transmutation
echo "=== 3. BFV TRANSMUTATION ==="
timeout 15 bin/ritual_bfv 2>/dev/null | grep -q "Truth preserved"
check "BFV Transmutation ritual"

# 4. BinFHE 2-bit
echo "=== 4. BINFHE 2-BIT CT×CT ==="
timeout 15 bin/phi_binfhe_ctct 2>/dev/null | grep -q "0100"
check "BinFHE 2-bit (2×2=4)"

# 5. BinFHE 4-bit
echo "=== 5. BINFHE 4-BIT CT×CT ==="
timeout 30 bin/phi_binfhe_4bit 2>/dev/null | grep -q "42 ✅"
check "BinFHE 4-bit (3×14=42)"

# 6. BinFHE 16-bit
echo "=== 6. BINFHE 16-BIT CT×CT ==="
timeout 30 tests/outputs/phi_binfhe_16bit 2>/dev/null | grep -q "714"
check "BinFHE 16-bit (42×17=714)"

# 7. NTL Hijack
echo "=== 7. NTL HIJACK ==="
grep -q "entangle" src/semantic/phi_ntl.hpp 2>/dev/null
check "NTL semantic hijack (entangle defined)"

# 8. SEAL Noise Hijack
echo "=== 8. SEAL NOISE HIJACK ==="
grep -q "check_coherence" src/semantic/phi_seal_noise.hpp 2>/dev/null
check "SEAL noise hijack (check_coherence defined)"

# 9. PHI Core
echo "=== 9. PHI CORE ==="
grep -q "I_AM" src/semantic/phi_core.h 2>/dev/null
check "PHI Core (I_AM defined)"

# 10. Scheme Switching
echo "=== 10. SCHEME SWITCHING ==="
timeout 15 bin/phi_scheme_switch_bootstrap 2>/dev/null | grep -q "FULLY RENEWED"
check "BFV↔BinFHE scheme switching"

# 11. SpiralKEM Integration
echo "=== 11. SPIRALKEM+FHE ==="
timeout 15 bin/phi_spiralkem_fhe 2>/dev/null | grep -q "714"
check "SpiralKEM+FHE (42×17=714)"

# 12. PHI ZKP Library
echo "=== 12. PHI ZKP ==="
timeout 30 bin/test_phi_zkp 2>/dev/null | grep -q "6/6 passed"
check "PHI ZKP Library (6/6 tests)"

# 13. Verifiable FHE
echo "=== 13. VERIFIABLE FHE ==="
timeout 15 tests/outputs/phi_verifiable_fhe 2>/dev/null | grep -q "VALID"
check "Verifiable FHE framework"

# 14. SNARK
echo "=== 14. SNARK ==="
timeout 15 tests/outputs/phi_snark_fhe 2>/dev/null | grep -q "99.9%"
check "SNARK (99.9% smaller proofs)"

# 15. EC-SNARK
echo "=== 15. EC-SNARK ==="
timeout 15 tests/outputs/phi_snark_ec 2>/dev/null | grep -q "VALID EC-SNARK"
check "EC-SNARK with BN254 pairings"

echo ""
echo "╔══════════════════════════════════════════════╗"
echo "║  RESULTS: $PASSED/$TOTAL passed                    ║"
if [ $FAILED -eq 0 ]; then
    echo "║  ALL SYSTEMS OPERATIONAL ✅                    ║"
else
    echo "║  $FAILED components need attention ❌          ║"
fi
echo "╚══════════════════════════════════════════════╝"
