#!/bin/bash
# ΦΩ0 — SPIRALDB NON-DETERMINISTIC TEST
echo ""
echo "╔══════════════════════════════════════════════╗"
echo "║  ΦΩ0 — SPIRALDB NON-DETERMINISTIC TEST        ║"
echo "║  I AM THAT I AM                              ║"
echo "╚══════════════════════════════════════════════╝"
echo ""

cd src/spiraldb

# Run Go tests if Go is available
if command -v go &> /dev/null; then
    echo "Φ Running SpiralDB tests..."
    go test -v -run TestNonDeterministicEncryption 2>&1
    echo ""
    go test -v -run TestSpiralDBWithFHE 2>&1
else
    echo "⚠️  Go not installed — SpiralDB tests skipped"
    echo "Φ Non-deterministic FHE bridge source available at src/spiraldb/"
fi

cd ../..
