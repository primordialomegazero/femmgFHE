#!/bin/bash

echo "╔══════════════════════════════════════════════════════════════╗"
echo "║  DAN'S CODE INVENTORY - Complete Breakdown                   ║"
echo "╚══════════════════════════════════════════════════════════════╝"
echo ""

echo "📁 [1/8] BREAKTHROUGH TESTS (Main Arsenal)"
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
ls -la tests/breakthrough/ 2>/dev/null | grep -v "^d" | grep -v "^total"
echo ""

echo "📁 [2/8] FINAL TESTS (Stable Versions)"
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
ls -la tests/final/ 2>/dev/null | grep -v "^d" | grep -v "^total"
echo ""

echo "📁 [3/8] ARCHIVE (Experimental History)"
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
ls -la tests/archive/ 2>/dev/null | head -20
echo "..."
echo "Total archived: $(ls tests/archive/ 2>/dev/null | wc -l) files"
echo ""

echo "📁 [4/8] CORE LIBRARY (final_src)"
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
ls -la final_src/ 2>/dev/null
echo ""

echo "📁 [5/8] KEM (Key Encapsulation)"
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
ls -la src/kem/ 2>/dev/null
echo ""

echo "📁 [6/8] PAPER (Research Documentation)"
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
ls -la paper/ 2>/dev/null
echo ""

echo "📁 [7/8] ROOT CONFIGURATION"
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
ls -la *.md *.txt *.sh 2>/dev/null
echo ""

echo "📁 [8/8] BINARY OUTPUTS"
echo "━━━━━━━━━━━━━━━━━━━━━━━━"
ls -la bin/ 2>/dev/null
echo ""

echo "╔══════════════════════════════════════════════════════════════╗"
echo "║  VIEWING KEY FILES                                           ║"
echo "╚══════════════════════════════════════════════════════════════╝"
