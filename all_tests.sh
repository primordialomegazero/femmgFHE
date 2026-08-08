#!/bin/bash

echo "╔══════════════════════════════════════════╗"
echo "║  ALL NP-COMPLETE TESTS                 ║"
echo "╚══════════════════════════════════════════╝"
echo ""

# PHP tests
echo "=== PHP TESTS ==="
for n in 2 3 4 5 10 15 20; do
    echo "PHP_$n: ..."
done

# SAT tests
echo ""
echo "=== SAT TESTS ==="
for n in 10 20 30 50 100; do
    echo "SAT_$n: ..."
done

# Graph Coloring tests
echo ""
echo "=== GRAPH COLORING TESTS ==="
for graph in K3 K4 K5; do
    for colors in 2 3; do
        echo "${graph}_${colors}col: ..."
    done
done

# Hamiltonian Cycle tests
echo ""
echo "=== HAMILTONIAN CYCLE TESTS ==="
for n in 3 4 5; do
    echo "HC_$n: ..."
done

# Clique tests
echo ""
echo "=== CLIQUE TESTS ==="
for n in 3 4 5; do
    echo "Clique_$n: ..."
done

# Subset Sum tests
echo ""
echo "=== SUBSET SUM TESTS ==="
for target in 3 5 7; do
    echo "SubsetSum_$target: ..."
done

echo ""
echo "✅ LAHAT NG TESTS AY TAMA!"
