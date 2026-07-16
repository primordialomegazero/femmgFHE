#!/bin/bash
# ΦΩ0 — Source Tree Audit & Cleanup
# Identifies: duplicates, test variants, unused files

echo "=============================================="
echo "  SOURCE TREE AUDIT"
echo "=============================================="
echo ""

echo "=== CORE FILES (actively used) ==="
echo "  src/core/zans_production_lib.h        — ZANS Engine v3.1.1"
echo "  src/core/phi_pinky_swear.cpp          — Step 5: Overflow Detection"
echo "  src/core/phi_true_divine_1M.cpp       — THE Breakthrough"
echo "  src/core/phi_precomputed_values.h     — Required by 1M"
echo "  src/core/phi_femmg_io_ultimate.cpp    — iO Suite"
echo "  src/core/phi_catchmeifyouKEM.cpp      — 80B PQ KEM"
echo "  src/core/phi_covenant_vault.cpp       — Secure Storage"
echo "  src/core/phi_spiraldb_unified.cpp     — Encrypted DB"
echo "  src/core/phi_entangled_prime_zans.cpp — Prime ZANS"
echo "  src/core/phi_phantom_suite_v2.cpp     — Obfuscation Suite"
echo "  src/core/phi_riemann_golden_zans.cpp  — Theoretical"
echo "  src/core/phi_quantum_random.cpp       — Theoretical"
echo "  src/core/phi_transmutation_window.cpp — Theoretical"
echo "  src/core/phi_verifiable_fhe_v2.cpp    — Audit Trail"
echo "  src/core/phi_unified_auth.cpp         — Auth System"
echo ""

echo "=== FILES TO DELETE (variants/duplicates) ==="
echo "  The following are older/test variants superseded by main files:"
echo ""

FILES_TO_DELETE=(
    # True Divine variants (superseded by 1M)
    "src/core/phi_true_divine_10k.cpp"
    "src/core/phi_true_divine_100k.cpp"
    "src/core/phi_true_divine_10k_perstep.cpp"
    
    # ZANS cross-library variants (results in docs/)
    "src/core/phi_zans_ckks.cpp"
    "src/core/phi_zans_ckks_100k.cpp"
    "src/core/phi_zans_ckks_10M.cpp"
    "src/core/phi_zans_helib_10M.cpp"
    "src/core/phi_zans_helib_10k.cpp"
    "src/core/phi_zans_seal_100k.cpp"
    "src/core/phi_zans_seal_10M.cpp"
    "src/core/phi_zans_tfhe_test.c"
    
    # iO sub-components (all in ultimate now)
    "src/core/phi_femmg_io_eternal_zans.cpp"
    "src/core/phi_femmg_io_fractal.cpp"
    "src/core/phi_femmg_io_kilian_full.cpp"
    "src/core/phi_femmg_io_pow_fix.cpp"
    
    # Phantom variants
    "src/core/phi_phantom_suite_v2_security.cpp"
    
    # Fibonacci/prime variants (in lib now)
    "src/core/phi_fib_global_zans.cpp"
    "src/core/phi_prime_chaos_zans_v2.cpp"
    
    # Old test files
    "src/core/zans_v311_quick_test.cpp"
)

for f in "${FILES_TO_DELETE[@]}"; do
    if [ -f "$f" ]; then
        echo "  DELETE: $f"
        rm -f "$f"
    else
        echo "  NOT FOUND: $f"
    fi
done

echo ""
echo "=== BINARIES TO CLEAN ==="
echo "  Removing old test binaries..."
rm -f bin/phi_true_divine_10k_test
rm -f bin/phi_true_divine_test
rm -f bin/zans_v311_quick_test
rm -f bin/phi_catchmeifyouKEM_1bit
rm -f bin/phi_catchmeifyouKEM_80B
rm -f bin/phi_catchmeifyouKEM_80B_1k
rm -f bin/phi_catchmeifyouKEM_72B_1k
rm -f bin/phi_catchmeifyouKEM_64B_1k
rm -f bin/phi_catchmeifyouKEM_final
rm -f bin/phi_catchmeifyouKEM_micro
rm -f bin/phi_catchmeifyouKEM_noseed

echo ""
echo "=== DONE ==="
echo "Source tree cleaned. Ready for commit."
