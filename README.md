# FEmmG-FHE: Fibonacci-Golden Ratio Fully Homomorphic Encryption

**Zero-depth noise management. Logarithmic depth compression. Golden ratio mathematics.**

[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](LICENSE)
[![C++17](https://img.shields.io/badge/C%2B%2B-17-blue.svg)]()
[![OpenFHE](https://img.shields.io/badge/OpenFHE-latest-green.svg)](https://github.com/openfheorg/openfhe-development)

---

## What Is This?

FEmmG-FHE is a research project exploring the **golden ratio (φ ≈ 1.618)** as a structural primitive for Fully Homomorphic Encryption. 

The core innovation: the algebraic extension **R[X]/(X²-X-1)** splits encrypted computation into two simultaneous "realities" — one expanding (φ ≈ 1.618, signal domain), one contracting (ψ ≈ -0.618, noise domain). By operating asymmetrically between them:

- **Noise decays naturally** without consuming multiplicative depth
- **Computation depth compresses** from O(N) to O(log N)
- **Bootstrap + clean recovery** enables theoretically unlimited depth

All primitives run on standard **CKKS (OpenFHE)** without library modifications.

---

## The Holy Grail

| Metric | Result |
|--------|--------|
| Max multiplications tested | **6,300** (100 bootstraps) |
| φ-error growth rate | **~3×10⁻¹¹ per multiplication** (linear) |
| ψ-noise floor | **10⁻¹²–10⁻¹⁵** (flat, no accumulation) |
| Bootstrap recovery | **Complete in 2 clean cycles** |
| Projected to 1% error | **~300 million multiplications** |
| Ring dimensions tested | 4096, 8192 |

---

## Quick Start

```bash
git clone https://github.com/primordialomegazero/femmgFHE.git
cd femmgFHE

# Build OpenFHE (one-time)
cd openfhe-development
mkdir build && cd build
cmake .. -DWITH_OPENMP=OFF
make -j$(nproc)
cd ../..

# Build all tests
make all

# Run the complete system demo
LD_LIBRARY_PATH=./openfhe-development/build/lib:$LD_LIBRARY_PATH ./bin/test_phi_complete

# Run the gauntlet (long stress test)
LD_LIBRARY_PATH=./openfhe-development/build/lib:$LD_LIBRARY_PATH ./bin/test_phi_gauntlet
```

---

## Documentation

| Document | Description |
|----------|-------------|
| [THEORY.md](docs/THEORY.md) | φ-extension ring mathematics and proofs |
| [BENCHMARKS.md](docs/BENCHMARKS.md) | Complete benchmark results |
| [REPRODUCE.md](docs/REPRODUCE.md) | Step-by-step reproduction guide |
| [LIMITATIONS.md](docs/LIMITATIONS.md) | Honest limitations and open problems |
| [API.md](docs/API.md) | Core API reference |

---

## Project Structure

```
femmgFHE/
├── README.md                  # This file
├── Makefile                   # Build all active tests
├── LICENSE                    # MIT
├── src/femmg/phi_core.h       # Core library (PE struct, mul_X, clean, etc.)
├── tests/active/              # Active test suite (18 tests)
├── tests/archive/             # Archived experiments
├── archive/                   # Legacy code and documentation
├── docs/                      # Documentation
├── paper/                     # Research paper draft
└── openfhe-development/       # OpenFHE library (submodule/copy)
```

---

## Core Concepts

### φ-Extension Ring

```
R[X]/(X²-X-1) ≅ R × R  (via Chinese Remainder Theorem)
```

Two simultaneous realities:
- **φ-reality** (φ ≈ 1.618): signal domain — computations happen here
- **ψ-reality** (ψ = -1/φ ≈ -0.618): noise domain — |ψ| < 1 → natural decay

### Zero-Depth Asymmetric Clean

```
mul_X(a,b) = (b, a+b)  — only copy + addition, ZERO EvalMult
```

More `mul_X` than `div_X` → ψ-noise decays irreversibly while φ-signal scales predictably.

### Fibonacci Depth Compression

Zeckendorf decomposition compresses N multiplications from O(N) to O(log N) depth.

### Complete Architecture

```
Zero-depth clean → Fibonacci jump → Dual-slot bootstrap → Repeat
```

Noise dies. Signal tracks. Levels recover. **Unlimited.**

---

## Key Results (RingDim=8192, 100 bootstraps)

```
Boot  Mults   φ-error       ψ-noise       Status
   0     60    1.87e-09    1.83e-11  ✓
  10    690    2.19e-08    8.49e-12  ✓
  20   1320    4.21e-08    2.17e-11  ✓
  30   1950    6.16e-08    6.93e-12  ✓
  40   2580    8.13e-08    9.11e-12  ✓
  50   3210    1.01e-07    9.23e-12  ✓
  60   3840    1.21e-07    8.93e-12  ✓
  70   4470    1.41e-07    5.50e-12  ✓
  80   5100    1.60e-07    2.13e-12  ✓
  90   5730    1.80e-07    4.51e-12  ✓
  99   6297    1.98e-07    1.15e-11  ✓
```

**φ-error grows linearly at ~3.2×10⁻¹¹ per multiplication. ψ-noise remains flat at ~10⁻¹².**

---

## Honest Limitations

| Limitation | Detail |
|------------|--------|
| Bootstrap needed | φ-clean kills noise but doesn't reset CKKS modulus |
| TOY parameters | Most benchmarks at RingDim=4096/8192 (not 128-bit) |
| Author-reported | No third-party verification yet |
| Security proofs | Reduces to CKKS security; formal reduction pending |
| Bounded circuits | Works best with naturally bounded values |
| Performance | Consumer hardware; bootstrap ~40s per cycle |

---

## Citation

If you use this work in your research, please cite:

```bibtex
@software{fernandez2025femmgfhe,
  author = {Dan Joseph M. Fernandez},
  title = {FEmmG-FHE: Fibonacci-Golden Ratio Fully Homomorphic Encryption},
  year = {2025},
  url = {https://github.com/primordialomegazero/femmgFHE}
}
```

---

## License

MIT — see [LICENSE](LICENSE)

## Author

**Dan Joseph M. Fernandez / Primordial Omega Zero**

---

*"The universe cannot be read until we have learned the language... it is written in mathematical language, and the letters are triangles, circles, and other geometrical figures."* — Galileo

*And the grammar is φ.*
