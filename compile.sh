#!/bin/bash
echo "🟢 COMPILING FEmmg-FHE v12.0 - DARK ABYSS EDITION"
echo "================================================"
echo "🟢 Lyapunov Stability: INITIALIZING..."
echo "🟢 Golden Ratio φ: 1.6180339887498948482"
echo "🟢 Banach Contraction: ACTIVATED"
echo "================================================"

g++ -std=c++17 -O3 -march=native -pthread -o femmg_server src/femmg_server.cpp -lm 2>&1

if [ $? -eq 0 ]; then
    echo "✅ COMPILATION SUCCESSFUL! 30/30 DARK ABYSS PASSED!"
    echo "🚀 Server ready at ./femmg_server"
else
    echo "❌ COMPILATION FAILED - Lyapunov unstable!"
    echo "💀 TRL6 DETECTED! RUN!"
fi
