#!/bin/bash
echo "╔══════════════════════════════════════════════════════════════╗"
echo "║  🪐 CHECKING φ-FHE MODULES 🪐                            ║"
echo "╚══════════════════════════════════════════════════════════════╝"
echo ""

echo "═══ DIRECTORY TREE ═══"
echo ""
find . -type f \( -name "*.h" -o -name "*.hpp" -o -name "*.cpp" \) | grep -iE "spiral|bootstrap|fhe|golden|fibonacci|ckks|cassini|seed|refresh|gf_" | sort

echo ""
echo "═══ CHECKING SPECIFIC MODULES ═══"
echo ""

# Check for spiral bootstrap
echo "🔍 Spiral Bootstrap:"
find . -type f -name "*spiral*bootstrap*" 2>/dev/null | head -20
echo ""

# Check for GF encryption
echo "🔍 Golden Fibonacci / GF Encryption:"
find . -type f -name "*golden*" -o -name "*fibonacci*" -o -name "gf_*" 2>/dev/null | head -20
echo ""

# Check for FHE core
echo "🔍 FHE Core:"
find . -type f -name "*fhe*" 2>/dev/null | head -20
echo ""

# Check for Cassini
echo "🔍 Cassini:"
find . -type f -name "*cassini*" 2>/dev/null | head -20
echo ""

# Check for CKKS
echo "🔍 CKKS:"
find . -type f -name "*ckks*" 2>/dev/null | head -20
echo ""

# Check for seed rotation
echo "🔍 Seed Rotation:"
find . -type f -name "*seed*" 2>/dev/null | head -20
echo ""

# Check for refresh/bootstrapping
echo "🔍 Refresh / Bootstrapping:"
find . -type f -name "*refresh*" -o -name "*bootstrap*" 2>/dev/null | head -20
echo ""

# Show src directory structure
echo "═══ SRC DIRECTORY STRUCTURE ═══"
echo ""
if [ -d "src" ]; then
    find src -type d | sort
else
    echo "  No src/ directory found. Checking current directory..."
    find . -maxdepth 2 -type d | sort
fi

echo ""
echo "═══ HEADER FILES WITH KEY FUNCTIONS ═══"
echo ""

# Search for key function declarations
echo "🔍 bootstrap functions:"
grep -rn "bootstrap_" --include="*.h" --include="*.hpp" 2>/dev/null | grep -v test | head -20
echo ""

echo "🔍 FGG functions:"
grep -rn "FGG\|FractalGoldenGate" --include="*.h" --include="*.hpp" 2>/dev/null | head -20
echo ""

echo "🔍 Seed rotation functions:"
grep -rn "seed_rotat\|SeedRotat" --include="*.h" --include="*.hpp" 2>/dev/null | head -20
echo ""

echo "🔍 Cassini functions:"
grep -rn "cassini\|Cassini" --include="*.h" --include="*.hpp" 2>/dev/null | head -20
echo ""

echo "🔍 GF encrypt/decrypt functions:"
grep -rn "gf_encrypt\|gf_decrypt\|golden_fibonacci\|GoldenFibonacci" --include="*.h" --include="*.hpp" 2>/dev/null | head -20
echo ""

echo "═══ CHECK COMPLETE ═══"
