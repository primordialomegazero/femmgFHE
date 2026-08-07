#!/bin/bash
# 🪐 DIRECT COMPILE φ-DPLL — BYPASS CMAKE 🪐
set -e

echo "╔══════════════════════════════════════════════════════════════╗"
echo "║  🪐 DIRECT COMPILE φ-DPLL TESTS 🪐                       ║"
echo "╚══════════════════════════════════════════════════════════════╝"

cd ~/femmgFHE

# ═══════════════════════════════════════════
# FIND ALL RELEVANT HEADERS
# ═══════════════════════════════════════════
echo ""
echo "═══ FINDING HEADERS ═══"

# Find phi_stack.h and other core headers
INCLUDE_DIRS=""
for dir in unified-phi-stack src include; do
    if [ -d "$dir" ]; then
        INCLUDE_DIRS="$INCLUDE_DIRS -I./$dir"
        echo "  Include: ./$dir"
    fi
done

# Add subdirectories
for dir in $(find src -type d 2>/dev/null); do
    INCLUDE_DIRS="$INCLUDE_DIRS -I./$dir"
done

# ═══════════════════════════════════════════
# COMPILE test_phi_no_memo.cpp (SIMPLEST)
# ═══════════════════════════════════════════
echo ""
echo "═══ COMPILING test_phi_no_memo.cpp ═══"

TEST_FILE="tests/breakthrough/test_phi_no_memo.cpp"
OUTPUT_BIN="build/test_phi_no_memo"

if [ -f "$TEST_FILE" ]; then
    echo "  Source: $TEST_FILE ($(wc -c < $TEST_FILE) bytes)"
    
    # Show first 50 lines to understand structure
    echo ""
    echo "  ─── First 60 lines ───"
    head -60 "$TEST_FILE"
    echo "  ─── End preview ───"
    echo ""
    
    # Try to compile
    echo "  Compiling..."
    g++ -std=c++17 -O2 -o "$OUTPUT_BIN" "$TEST_FILE" $INCLUDE_DIRS -lm 2>&1 || {
        echo ""
        echo "  ⚠️  Compilation failed — showing errors above"
        echo "  Trying with more include paths..."
        
        # More aggressive include search
        ALL_INCLUDES=$(find . -type d -not -path './.git/*' -not -path './build/*' -not -path './openfhe-development/*' | sed 's/^/-I.\//' | tr '\n' ' ')
        g++ -std=c++17 -O2 -o "$OUTPUT_BIN" "$TEST_FILE" $ALL_INCLUDES -lm 2>&1 | head -30
    }
    
    if [ -x "$OUTPUT_BIN" ]; then
        echo ""
        echo "  ✅ Compilation successful!"
        echo ""
        echo "═══ RUNNING test_phi_no_memo ═══"
        timeout 30 "$OUTPUT_BIN" 2>&1 || echo "  (timeout or error)"
    fi
else
    echo "  ❌ $TEST_FILE not found"
fi

# ═══════════════════════════════════════════
# ALSO TRY: test_pigeonhole_verify.cpp
# ═══════════════════════════════════════════
echo ""
echo "═══ COMPILING test_pigeonhole_verify.cpp ═══"

TEST_FILE2="tests/breakthrough/test_pigeonhole_verify.cpp"
OUTPUT_BIN2="build/test_pigeonhole_verify"

if [ -f "$TEST_FILE2" ]; then
    echo "  Source: $TEST_FILE2 ($(wc -c < $TEST_FILE2) bytes)"
    echo "  Compiling..."
    
    ALL_INCLUDES=$(find . -type d -not -path './.git/*' -not -path './build/*' -not -path './openfhe-development/*' | sed 's/^/-I.\//' | tr '\n' ' ')
    g++ -std=c++17 -O2 -o "$OUTPUT_BIN2" "$TEST_FILE2" $ALL_INCLUDES -lm 2>&1 | head -30
    
    if [ -x "$OUTPUT_BIN2" ]; then
        echo ""
        echo "  ✅ Compilation successful!"
        echo ""
        echo "═══ RUNNING test_pigeonhole_verify ═══"
        timeout 30 "$OUTPUT_BIN2" 2>&1 || echo "  (timeout or error)"
    fi
else
    echo "  ❌ $TEST_FILE2 not found"
fi

echo ""
echo "═══ DONE ═══"

