#!/bin/bash
echo "╔══════════════════════════════════════════════╗"
echo "║  ΦΩ0 — AGGRESSIVE CLEANUP v1.0               ║"
echo "║  Removing wackshit, keeping goodshit         ║"
echo "╚══════════════════════════════════════════════╝"
echo ""

BACKUP_DIR="archive/cleanup_backup_$(date +%Y%m%d_%H%M%S)"
mkdir -p "$BACKUP_DIR"
echo "Backup: $BACKUP_DIR"
echo ""

# ============================================
# SRC/CORE — Keep only the goodshit
# ============================================
echo "[1/8] Cleaning src/core/..."

CORE_KEEP=(
    "zans_production_lib.h"
    "phi_zans_bfv.cpp"
    "phi_fib_zans.cpp"
    "phi_fib_zans_ctct.cpp"
)

CORE_WACK=(
    "banach_engine.h"
    "cerberus_unified.h"
    "femmg_operations.h"
    "femmg_operations_v2.h"
    "femmg_true_core.h"
    "fhe_core.h"
    "fhe_seal_bridge.h"
    "full_true_fhe.h"
    "lyapunov_fhe.h"
    "metaprogram.h"
    "phi_fhe_core.h"
    "triple_layer_fhe.h"
    "true_homomorphic.h"
    "true_poly_fhe.h"
)

for f in "${CORE_WACK[@]}"; do
    if [ -f "src/core/$f" ]; then
        mv "src/core/$f" "$BACKUP_DIR/"
        echo "  🗑️  $f → backup"
    fi
done

echo "  ✅ src/core/ cleaned"
echo ""

# ============================================
# SRC/BINFHE — Keep all, decide later on hybrid
# ============================================
echo "[2/8] Cleaning src/binfhe/..."
echo "  ℹ️  Keeping all BinFHE files (review later)"
echo ""

# ============================================
# SRC/KEM — Remove wrappers, keep SpiralKEM
# ============================================
echo "[3/8] Cleaning src/kem/..."

KEM_WACK=(
    "ml_kem_native.h"
    "ml_kem_wrapper.h"
    "phi_algo_merge.h"
    "phi_parallel_kem.h"
)

for f in "${KEM_WACK[@]}"; do
    if [ -f "src/kem/$f" ]; then
        mv "src/kem/$f" "$BACKUP_DIR/"
        echo "  🗑️  $f → backup"
    fi
done

echo "  ✅ src/kem/ cleaned"
echo ""

# ============================================
# SRC/ZKP — Keep all
# ============================================
echo "[4/8] Cleaning src/zkp/..."
echo "  ℹ️  Keeping all ZKP files"
echo ""

# ============================================
# SRC/SNARK — Keep all
# ============================================
echo "[5/8] Cleaning src/snark/..."
echo "  ℹ️  Keeping all SNARK files"
echo ""

# ============================================
# SRC/SEMANTIC — Remove entire folder
# ============================================
echo "[6/8] Cleaning src/semantic/..."
if [ -d "src/semantic" ]; then
    mv src/semantic "$BACKUP_DIR/semantic"
    echo "  🗑️  src/semantic/ → backup (whole folder)"
fi
echo ""

# ============================================
# SRC/TRANSMUTE — Keep only scheme switch
# ============================================
echo "[7/8] Cleaning src/transmute/..."

TRANSMUTE_WACK=(
    "phi_ckks_bootstrap_debug.cpp"
    "phi_ckks_workaround.cpp"
    "phi_ckks_transmute.cpp"
    "ritual_bfv.cpp"
)

for f in "${TRANSMUTE_WACK[@]}"; do
    if [ -f "src/transmute/$f" ]; then
        mv "src/transmute/$f" "$BACKUP_DIR/"
        echo "  🗑️  $f → backup"
    fi
done

echo "  ✅ src/transmute/ cleaned"
echo ""

# ============================================
# TESTS — Remove binaries
# ============================================
echo "[8/8] Cleaning tests/..."

TEST_WACK=(
    "all_ones_optimized.cpp"
    "phi_binfhe_4bit"
    "phi_binfhe_ctct"
    "phi_fib_zans"
    "phi_zans_bfv"
    "ritual_bfv"
    "test_phi_zkp"
)

for f in "${TEST_WACK[@]}"; do
    if [ -f "tests/$f" ]; then
        mv "tests/$f" "$BACKUP_DIR/"
        echo "  🗑️  $f → backup"
    fi
done

echo "  ✅ tests/ cleaned"
echo ""

# ============================================
# ROOT — Remove logs, temp files
# ============================================
echo "[EXTRA] Cleaning root..."
rm -f *.log
rm -f femmgFHE_quick_check.sh
echo "  🗑️  *.log files deleted"
echo ""

# ============================================
# SUMMARY
# ============================================
echo "╔══════════════════════════════════════════════╗"
echo "║  CLEANUP COMPLETE                            ║"
echo "╠══════════════════════════════════════════════╣"
echo "║  All wackshit moved to:                      ║"
echo "║  $BACKUP_DIR"
echo "╠══════════════════════════════════════════════╣"
echo "║  Goodshit retained. Ready for upgrade.       ║"
echo "║  Next: fix Makefile, rebuild, test           ║"
echo "╚══════════════════════════════════════════════╝"
