#!/bin/bash
echo "🔍 FEmmg-FHE Source Code Explorer"
echo "=================================="
echo ""

files=(
    "src/banach_engine.h"
    "src/golden_chaos.h"
    "src/femmg_fhe.h"
    "src/blackhole_fhe.h"
    "src/phi_parallel_kem.h"
    "src/spiral_db_lite.h"
    "src/zkp_fractal.h"
    "src/femmg_server.cpp"
)

for file in "${files[@]}"; do
    if [ -f "$file" ]; then
        echo "📄 $file"
        echo "---"
        wc -l "$file"
        echo ""
    fi
done
