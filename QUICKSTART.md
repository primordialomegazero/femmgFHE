# Quick Start Guide

## 1. Install Dependencies

```bash
# Ubuntu/Debian
sudo apt update
sudo apt install -y libntl-dev libgmp-dev g++ make

# Verify
g++ --version
dpkg -l | grep libntl
```

## 2. Clone Repository

```bash
git clone https://github.com/primordialomegazero/femmgFHE.git
cd femmgFHE
```

## 3. Build

```bash
# Build all tests
make all

# Or build individual test
make test_complete_pipeline
```

## 4. Run Tests

```bash
# Complete pipeline (FHE + iO + Quantum)
make run-pipeline

# Full Adder
make run-full-adder

# 4-bit Ripple Adder
make run-4bit-adder

# 2048-bit + Quantum
make run-2048-quantum
```

## 5. Run Directly (without make)

```bash
# After building, binaries are in build/
./build/test_complete_pipeline
./build/test_full_adder_complete
./build/test_4bit_adder
./build/test_2048_full_quantum
```

## 6. Clean

```bash
# Remove build artifacts
make clean

# Remove everything including results
make clean-all
```

## Expected Output

```
COMPLETE PIPELINE TEST
======================

PHASE 1: FHE CORE
  Encrypt(0) → 0 ✓
  Encrypt(1) → 1 ✓
  10 NOT gates → 1 ✓

PHASE 2: iO STANDALONE
  iO XOR: ALL PASS ✓

PHASE 3: QUANTUM STANDALONE
  Quantum CNOT: ALL PASS ✓

PHASE 4: FUSED PIPELINE
  Pipeline result: 0 ✓

PHASE 5: UNLIMITED DEPTH
  10K ops, 0 errors ✓
```

## Troubleshooting

### "libntl not found"
```bash
sudo apt install libntl-dev
```

### "g++: command not found"
```bash
sudo apt install g++
```

### "make: command not found"
```bash
sudo apt install make
```

### Compilation errors
- Ensure C++17 support: `g++ --version` (should be 9+)
- Ensure NTL is installed: `dpkg -l | grep ntl`

## Requirements Summary

| Dependency | Version | Purpose |
|-----------|---------|---------|
| GCC/G++ | 9+ | Compiler (C++17) |
| NTL | 11.5+ | Number theory library |
| GMP | 6.2+ | Big integer arithmetic |
| Make | 4.2+ | Build system |
| Linux | Ubuntu 20.04+ | OS (uses -march=native) |
