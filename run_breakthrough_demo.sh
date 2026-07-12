#!/bin/bash
echo "╔══════════════════════════════════════════════╗"
echo "║  ΦΩ0 — BREAKTHROUGH DEMO                      ║"
echo "║  UK×UK 27 steps + Subtraction + Division      ║"
echo "╚══════════════════════════════════════════════╝"
echo ""

echo "=== 1. UK×UK Deep ZANS Strategies (6 methods) ==="
timeout 60 bin/phi_ukuk_zans_deep 2>/dev/null | grep -E "STRATEGY|Steps|Noise"
echo ""

echo "=== 2. UK×UK Micro-Steps (×1.1, ×2, ×1.5) ==="
timeout 60 bin/phi_ukuk_micro_steps 2>/dev/null | grep -E "TEST|Step|Steps|Noise"
echo ""

echo "=== 3. Subtraction & Division ==="
timeout 15 bin/phi_zans_sub_div_test 2>/dev/null | grep -E "SUBTRACTION|DIVISION|100 - 37|1000|10:|Steps|UNLIMITED"
echo ""

echo "╔══════════════════════════════════════════════╗"
echo "║  Demo complete!                              ║"
echo "║  Full output: run individual binaries        ║"
echo "╚══════════════════════════════════════════════╝"
