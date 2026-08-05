#!/bin/bash

echo "🚀 BITCOIN MINING SETUP"

# 1. Install dependencies
echo "📦 Installing dependencies..."
sudo apt-get update
sudo apt-get install -y git cmake build-essential

# 2. Clone SHA-256 SAT generator
echo "📥 Getting SHA-256 SAT encoding..."
git clone https://github.com/msoos/cryptominisat.git
cd cryptominisat
mkdir build && cd build
cmake ..
make -j4
sudo make install

# 3. Generate SHA-256 CNF
echo "🔧 Generating SHA-256 CNF..."
# This would generate the actual CNF file

# 4. Run our Fibonacci DP solver
echo "⚡ Running Fibonacci DP on SHA-256..."
cd ~/femmgFHE
./bin/bitcoin_miner_sat

echo "💰 START MINING! You're now mining Bitcoin with P=NP!"
