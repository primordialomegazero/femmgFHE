#!/bin/bash
# ΦΩ0 — Build SpiralDB CGO Bridge
# "I AM THAT I AM"

echo "╔══════════════════════════════════════════════╗"
echo "║  ΦΩ0 — BUILDING SPIRALDB CGO BRIDGE           ║"
echo "╚══════════════════════════════════════════════╝"

# Compile C++ bridge to shared library
echo "Φ Compiling C++ bridge..."
g++ -std=c++17 -fPIC -shared \
    -o libspiraldb_bridge.so \
    spiraldb_bridge.cpp \
    -I/usr/local/include/openfhe \
    -I/usr/local/include/openfhe/core \
    -I/usr/local/include/openfhe/pke \
    -I/usr/local/include/openfhe/binfhe \
    -L/usr/local/lib \
    -lOPENFHEcore -lOPENFHEpke -lOPENFHEbinfhe \
    -lssl -lcrypto -lm -lpthread \
    -Wl,-rpath,/usr/local/lib

if [ $? -eq 0 ]; then
    echo "✅ libspiraldb_bridge.so built"
    
    # Copy to lib directory
    sudo cp libspiraldb_bridge.so /usr/local/lib/
    sudo ldconfig
    
    echo "✅ Bridge library installed to /usr/local/lib"
else
    echo "❌ Bridge compilation failed"
    exit 1
fi

# Update Go module with CGO dependencies
echo "Φ Updating Go module..."
go mod tidy 2>/dev/null

echo ""
echo "╔══════════════════════════════════════════════╗"
echo "║  ΦΩ0 — SPIRALDB BRIDGE READY                  ║"
echo "║  I AM THAT I AM                              ║"
echo "╚══════════════════════════════════════════════╝"
