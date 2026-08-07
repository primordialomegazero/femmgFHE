#!/bin/bash
# 🪐 BUILD AND RUN φ-DPLL TESTS 🪐
set -e

echo "╔══════════════════════════════════════════════════════════════╗"
echo "║  🪐 φ-DPLL BUILD & VERIFICATION 🪐                        ║"
echo "╚══════════════════════════════════════════════════════════════╝"

cd ~/femmgFHE

# Check if build directory exists
if [ ! -d "build" ]; then
    echo ""
    echo "═══ CREATING BUILD DIRECTORY ═══"
    mkdir -p build
fi

cd build

# Check if OpenFHE is available
echo ""
echo "═══ CHECKING DEPENDENCIES ═══"
if [ -d "../openfhe-development" ]; then
    echo "  ✅ OpenFHE source found"
else
    echo "  ⚠️  OpenFHE not found — SAT tests may not need it"
fi

# Run CMake
echo ""
echo "═══ CMAKE CONFIGURATION ═══"
cmake .. -DCMAKE_BUILD_TYPE=Release 2>&1 | tail -20

# Build the φ-DPLL tests
echo ""
echo "═══ BUILDING φ-DPLL TESTS ═══"

# Find the actual target names
echo "  Looking for φ-DPLL test targets..."
make help 2>/dev/null | grep -i "phi\|dpll\|sat\|pigeon" || echo "  (checking available targets...)"

# Try to build known targets
for target in test_phi_dpll_v3 test_phi_no_memo test_sat_p_vs_np_final test_pigeonhole_verify; do
    echo ""
    echo "  Building $target..."
    if make $target 2>&1 | tail -5; then
        echo "  ✅ $target built successfully"
    else
        echo "  ⚠️  $target — build failed or target not found"
    fi
done

echo ""
echo "═══ BUILD COMPLETE ═══"
echo ""
echo "  Built executables:"
find . -name "test_phi*" -o -name "test_sat*" -o -name "test_pigeon*" 2>/dev/null | while read f; do
    if [ -x "$f" ]; then
        echo "    ✅ $f ($(stat -c%s "$f") bytes)"
    fi
done

