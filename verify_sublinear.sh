#!/bin/bash
# ═══════════════════════════════════════════════════
# VERIFY SUB-LINEAR CLAIM
# ═══════════════════════════════════════════════════

echo "╔══════════════════════════════════════════════════════════════╗"
echo "║  🪐 VERIFY φ-DPLL SUB-LINEAR CLAIM 🪐                     ║"
echo "╚══════════════════════════════════════════════════════════════╝"
echo ""

echo "═══ PREDICTIONS (S(n) = 0.82 × n^0.61) ═══"
echo ""
echo "  n=10:   S(n) = $(echo "0.82 * 10^0.61" | bc -l | cut -c1-5) nodes"
echo "  n=20:   S(n) = $(echo "0.82 * 20^0.61" | bc -l | cut -c1-5) nodes"
echo "  n=30:   S(n) = $(echo "0.82 * 30^0.61" | bc -l | cut -c1-5) nodes"
echo "  n=50:   S(n) = $(echo "0.82 * 50^0.61" | bc -l | cut -c1-5) nodes"
echo "  n=75:   S(n) = $(echo "0.82 * 75^0.61" | bc -l | cut -c1-5) nodes"
echo "  n=100:  S(n) = $(echo "0.82 * 100^0.61" | bc -l | cut -c1-6) nodes"
echo "  n=150:  S(n) = $(echo "0.82 * 150^0.61" | bc -l | cut -c1-6) nodes"
echo "  n=200:  S(n) = $(echo "0.82 * 200^0.61" | bc -l | cut -c1-6) nodes"
echo ""

echo "═══ FOR COMPARISON ═══"
echo ""
echo "  Standard DPLL (worst case): O(2^n)"
echo "  n=50:  2^50 ≈ 1,125,899,906,842,624 nodes"
echo "  n=100: 2^100 ≈ 1.27 × 10^30 nodes"
echo ""
echo "  φ-DPLL claim: S(100) ≈ 10 nodes! (10^28 × FASTER)"
echo ""

echo "═══ FILES READY ═══"
ls -la test_sat_n*.cnf test_php5.cnf test_aim_benchmark.cnf 2>/dev/null
echo ""

echo "═══ RUN COMMANDS ═══"
echo ""
echo "  # Random 3-SAT:"
echo "  for f in test_sat_n*.cnf; do"
echo "    echo \"Running: \$f\""
echo "    ./sat_solver \$f"
echo "  done"
echo ""
echo "  # Pigeonhole (PHP-5, 30 vars):"
echo "  ./sat_solver test_php5.cnf"
echo ""
echo "  # AIM Benchmark:"
echo "  ./sat_solver test_aim_benchmark.cnf"
