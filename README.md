# femmgFHE

Fibonacci-based Fully Homomorphic Encryption research implementation.

## Overview

This repository contains an experimental implementation of a fully homomorphic encryption (FHE) scheme based on the golden ratio structure over finite fields. The scheme uses the identity φ² = φ+1 for automatic relinearization and noise management.

## Prerequisites

- Linux (Ubuntu 20.04+ or compatible)
- GCC/G++ 9+ (C++17 support required)
- NTL (Number Theory Library) 11.5+
- GMP (GNU Multiple Precision Arithmetic Library)

### Installing Dependencies

```bash
# Ubuntu/Debian
sudo apt update
sudo apt install -y libntl-dev libgmp-dev g++ make

# Verify installation
g++ --version
dpkg -l | grep libntl
```

## Building

### Core Library

The project is header-only. Include the necessary headers in your project:

```cpp
#include "src/fhe/golden_fibonacci_fhe_v5.h"
#include "src/io/golden_fibonacci_io_v2.h"
#include "src/quantum/golden_fibonacci_quantum_v2.h"
```

### Compiling Tests

```bash
# FHE Core tests
g++ -std=c++17 -O3 -march=native -I/usr/include tests/test_complete_pipeline.cpp -o test_pipeline -lntl -lgmp -lm

# iO tests
g++ -std=c++17 -O3 -march=native -I/usr/include tests/test_full_adder_complete.cpp -o test_full_adder -lntl -lgmp -lm

# Quantum tests
g++ -std=c++17 -O3 -march=native -I/usr/include tests/test_2048bit_full_adder_quantum.cpp -o test_2048_quantum -lntl -lgmp -lm

# Stress tests
g++ -std=c++17 -O3 -march=native -I/usr/include tests/test_257bit_100k_stress.cpp -o test_257_stress -lntl -lgmp -lm
```

## Project Structure

```
femmgFHE/
├── src/
│   ├── fhe/
│   │   ├── golden_fibonacci_fhe.h      # Core FHE (original)
│   │   ├── golden_fibonacci_fhe_v2.h   # Optimized (pre-computed)
│   │   ├── golden_fibonacci_fhe_v3.h   # Pre-allocated buffers
│   │   ├── golden_fibonacci_fhe_v4.h   # Toggle-optimized NOT
│   │   └── golden_fibonacci_fhe_v5.h   # Side-channel resistant
│   ├── io/
│   │   └── golden_fibonacci_io_v2.h    # iO (truth table + circuit)
│   ├── quantum/
│   │   └── golden_fibonacci_quantum_v2.h # Quantum gates
│   ├── golden_lucas.h                   # Lucas number computation
│   ├── golden_prng.h                    # Golden angle PRNG
│   └── golden_equidistributed.h         # Noise generation
├── tests/
│   ├── test_complete_pipeline.cpp       # Full pipeline test
│   ├── test_full_adder_complete.cpp     # Full adder (sum + cout)
│   ├── test_4bit_ripple_adder.cpp       # 4-bit ripple adder
│   └── test_2048bit_full_adder_quantum.cpp # 2048-bit + quantum
├── theorems/
│   ├── theorem6_rlwe_formal.cpp         # RLWE reduction
│   ├── theorem7_general_induction.cpp   # Unlimited depth proof
│   └── theorem_geometric.cpp            # Geometric properties
├── results/
│   ├── complete_data.txt                # Complete empirical data
│   ├── RESULTS_257BIT.md               # 257-bit results
│   ├── RESULTS_1024BIT.md              # 1024-bit results
│   └── RESULTS_2048BIT_FINAL.md        # 2048-bit results
├── formalproof.md                       # Formal mathematical proofs
├── informalproof.md                     # Informal explanation
└── FORMAL_PROOF_COMPLETE.md            # Complete 60/60 proof document
```

## Usage

### Basic FHE Operations

```cpp
#include "src/fhe/golden_fibonacci_fhe_v5.h"

int main() {
    // Initialize with 257-bit modulus
    NTL::ZZ Q = NTL::to_ZZ("115792089237316195423570985008687907853269984665640564039457584007913129640731");
    
    golden_fhe_v5::FibonacciFHEV5 fhe(Q, 42);
    
    // Encrypt
    auto ct0 = fhe.encrypt(false);
    auto ct1 = fhe.encrypt(true);
    
    // Decrypt
    bool bit0 = fhe.decrypt(ct0);  // false
    bool bit1 = fhe.decrypt(ct1);  // true
    
    // Homomorphic operations
    auto nand_result = fhe.nand_gate(ct0, ct1);
    auto not_result = fhe.not_gate(ct1);
    
    return 0;
}
```

### iO Operations

```cpp
#include "src/io/golden_fibonacci_io_v2.h"

int main() {
    NTL::ZZ Q = NTL::to_ZZ("...");
    GoldenFibonacciIOV2::FibonacciIOV2 io(Q, 42);
    
    // Build circuit
    io.obfuscate_circuit_begin(2);
    int g1 = io.circuit_add_nand(0, 1);
    int g2 = io.circuit_add_nand(0, g1);
    int g3 = io.circuit_add_nand(1, g1);
    int xor_out = io.circuit_add_nand(g2, g3);
    io.add_output(xor_out);
    
    // Evaluate
    std::vector<bool> input = {false, true};
    bool result = io.evaluate(input);
    
    return 0;
}
```

### Quantum Operations

```cpp
#include "src/quantum/golden_fibonacci_quantum_v2.h"

int main() {
    NTL::ZZ Q = NTL::to_ZZ("...");
    GoldenFibonacciQuantumV2::FusedQuantumFHEV2 quantum(Q, 42);
    
    auto ctrl = fhe.encrypt(true);
    auto tgt = fhe.encrypt(false);
    
    // CNOT gate
    auto result = quantum.cnot(ctrl, tgt);
    
    return 0;
}
```

## Testing

### Running All Tests

```bash
# Core pipeline
./test_pipeline

# Full adder
./test_full_adder

# 4-bit ripple adder
./test_4bit_adder

# 2048-bit + quantum
./test_2048_quantum
```

### Stress Tests

```bash
# 257-bit 100K NAND (takes ~27 minutes)
./test_257_stress

# 1024-bit 100K NAND (takes ~94 minutes)
./test_1024_stress

# 2048-bit 100K NAND (takes ~114 minutes)
./test_2048_stress
```

## Results

### Empirical Results Summary

| Test | Result | Errors | Ops/sec |
|------|--------|--------|---------|
| 32-bit 1M NAND | PASS | 0 | 168 |
| 257-bit 100K NAND | PASS | 0 | 62 |
| 1024-bit 100K NAND | PASS | 0 | 17.8 |
| 2048-bit 100K NAND | PASS | 0 | 14.7 |
| 4-bit Ripple Adder | 256/256 PASS | 0 | - |
| Full Adder | 8/8 PASS | 0 | - |
| 2-bit Comparator | 16/16 PASS | 0 | - |
| CNOT | 4/4 PASS | 0 | - |

## Documentation

- [Formal Proof](formalproof.md) - Mathematical proofs for all theorems
- [Informal Proof](informalproof.md) - Conceptual explanation
- [Complete Proof](FORMAL_PROOF_COMPLETE.md) - Full 60/60 proof document
- [Results](results/) - Empirical test results

## Known Limitations

- Statistical verification uses 100K samples (larger sample sizes in progress)
- 2048-bit 1M NAND test is ongoing
- Side-channel analysis is preliminary
- No formal security reduction to ideal lattices (RLWE only)

## License

This project is for research purposes. See [LICENSE.md](LICENSE.md) for details.

## Contributing

This is a research project. For issues or questions, please open an issue in the repository.

## Disclaimer

This is an experimental implementation. It has not been audited and should not be used in production systems without thorough review.
