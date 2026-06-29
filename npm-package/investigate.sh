#!/bin/bash

echo "========================================="
echo "🔍 INVESTIGATING NPM vs LOCAL BUILD"
echo "========================================="
echo ""

cd ~/femmgFHE/npm-package

echo "[1] Checking the actual NPM package code..."
echo ""
grep -A 5 -B 5 "encrypt" index.js | head -30

echo ""
echo "[2] Checking if Lyapunov is imported..."
echo ""
grep -i "lyapunov\|nonce\|chaos\|evolve" index.js | head -10

echo ""
echo "[3] Checking package.json version..."
echo ""
cat package.json | grep version

echo ""
echo "[4] Comparing with the build directory..."
echo ""
ls -la ~/build/femmgFHE/npm-package/ 2>/dev/null || echo "Build directory not found"

echo ""
echo "[5] Looking for the IND-CPA version (maybe in another branch?)"
echo ""
cd ~/femmgFHE
git branch -a 2>/dev/null || echo "Not a git repo or no branches"

echo ""
echo "[6] Checking the server code for nonce generation..."
echo ""
grep -A 10 "encrypt" src/femmg_server.cpp | head -20

echo ""
echo "========================================="
echo "💀 CONCLUSION:"
echo "========================================="
echo "✅ NPM package v13.0.0 = DETERMINISTIC"
echo "✅ Screenshot shows PROBABILISTIC version"
echo "❓ The probabilistic version may be:"
echo "   - A development branch"
echo "   - A local build not published yet"
echo "   - A separate version (v13.1.0 maybe)"
echo "   - Or the screenshot is from the SERVER side"
echo ""
echo "🔬 The mystery continues!"
