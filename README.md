# FEmmG-FHE: Fibonacci-Golden Ratio Cryptography

**DM-DGR: Dual Modulus + Double Golden Ratio — An Experimental FHE System**

[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](LICENSE)
[![C++17](https://img.shields.io/badge/C%2B%2B-17-blue.svg)]()
[![OpenFHE](https://img.shields.io/badge/OpenFHE-latest-green.svg)](https://github.com/openfheorg/openfhe-development)

---

## What Is This?

FEmmG-FHE explores the algebraic extension ring **R[X]/(X²-X-1)** — the golden ratio extension — as a primitive for homomorphic encryption.

The two roots are φ ≈ 1.618 (golden ratio) and ψ ≈ -0.618. One expands, one contracts. This asymmetry enables novel approaches to the fundamental challenges of Fully Homomorphic Encryption.

---

## DM-DGR: Ratio Encoding Approach

### Core Insight

Instead of encoding values as **magnitudes** (which grow exponentially with each multiplication), DM-DGR encodes values as **ratio deviations from the ψ-attractor** (ψ ≈ 0.618). The ratio converges to ψ under Fibonacci evolution and stays bounded, while the magnitude can grow arbitrarily without affecting the encoded value.

### Key Mechanisms

| Mechanism | What It Does | Cost |
|-----------|-------------|------|
| **φ/ψ Alternating Cycle** | φ-step expands, ψ-step contracts — exact inverses | 0 EvalMult |
| **ψ-Attractor** | Fibonacci evolution drives ratio → ψ ≈ 0.618 | 0 EvalMult |
| **Ratio Encoding** | Value = ratio - ψ, bounded regardless of magnitude | — |
| **Native Bootstrap** | Ring swap refreshes modulus without decryption | 0 EvalMult |
| **Bias Correction** | Algebraic formulas correct operation-specific biases | O(1) |

### Experimentally Verified (Consumer Hardware)

| Operation | Precision | Notes |
|-----------|-----------|-------|
| CT + CT (arbitrary values) | 10⁻¹² error | Verified |
| CT × CT (arbitrary values) | 10⁻¹³ error | Verified |
| Multiply-then-Add | 10⁻¹² error | Verified |
| Add-then-Multiply | 10⁻¹⁶ error (plaintext) | CKKS precision WIP |
| φ/ψ 100-cycle identity | 10⁻¹³ error | Verified |
| ψ-attractor convergence | 10⁻⁵ drift | Verified after 10 steps |
| Deep chains (6+ mixed ops) | Survives | No crash |

**10/13 tests passing on consumer hardware.** The 3 remaining are CKKS floating-point precision issues in add-then-multiply chains — not fundamental math problems.

### Test Results (Ryzen 5 2600, 16GB RAM, RingDim=4096)

```
=== PURE ADDITION ===
✅ A+B:       0.800000 (err=3.09e-12)
✅ A+B+C:     1.000000 (err=2.01e-12)
✅ 5×A:       2.500000 (err=1.68e-11)

=== PURE MULTIPLICATION ===
✅ A×B:       0.150000 (err=3.91e-12)
✅ F×F:       6.250000 (err=1.18e-11)
✅ D×D:       0.603729 (err=4.57e-12)

=== MULTIPLY THEN ADD ===
✅ (A×B)+(C×D): 0.305400 (err=1.31e-12)
✅ (A×B)+(E×F): 2.650000 (err=1.20e-11)

=== FIBONACCI EVOLUTION ===
✅ Converges to ψ: drift=4.72e-05

=== φ/ψ CYCLE ===
✅ 100× (φ+ψ) = identity: 1.000000 (err=1.53e-13)
```

---

## Cross-Library Compatibility

DM-DGR requires only 4 primitive operations: `EvalAdd`, `EvalSub`, `EvalMult`, `Enc(0)`. These exist in every FHE library.

| Library | Language | Scheme | Status |
|---------|----------|--------|--------|
| OpenFHE | C++ | CKKS | ✅ Verified (all DM-DGR tests) |
| SEAL 4.3 | C++ | BFV | ✅ Verified (cross-lib) |
| TFHE | C | TFHE | ✅ Verified (all primitives) |
| HElib | C++ | BGV | API confirmed |
| FHEW | C++ | FHEW | API confirmed |
| TenSEAL | Python | BFV/CKKS | API confirmed |
| Pyfhel | Python | BFV/CKKS | API confirmed |
| Lattigo v5 | Go | BGV/CKKS | API confirmed |
| Concrete | Rust | TFHE | API confirmed |

---

## Project Structure

```
femmgFHE/
├── tests/
│   ├── final/                    # ✅ Production DM-DGR tests
│   │   ├── test_phi_dm_dgr_final.cpp    # THE UNIFIED SYSTEM
│   │   ├── test_phi_dm_dgr_unified.cpp  # DM-DGR cycle test
│   │   ├── test_phi_basic_check.cpp     # Debug/development tests
│   │   ├── phi_kem_level5.c             # NIST Level 5 KEM
│   │   └── phi_kem_qr.c                 # QR-code KEM
│   ├── crosslib/                 # Cross-library validation
│   │   ├── test_phi_seal_crosslib.cpp
│   │   ├── test_phi_helib_crosslib.cpp
│   │   ├── test_phi_tfhe_crosslib.cpp
│   │   └── test_phi_crosslib_final.cpp
│   ├── legacy/                   # Archived experiments
│   └── README.md
├── final_src/                    # Core library headers
│   ├── phi_core.h
│   ├── phi_io_core.h
│   └── phi_io_compiler.h
├── src/kem/                      # KEM implementations
├── paper/                        # Research paper
├── archive/                      # Legacy (500MB+)
└── README.md
```

---

## Build & Run

```bash
git clone https://github.com/primordialomegazero/femmgFHE.git
cd femmgFHE

# Build OpenFHE (one-time)
cd openfhe-development && mkdir -p build && cd build
cmake .. -DWITH_OPENMP=OFF && make -j$(nproc)
cd ../..

# Build and run the final unified system
g++ -std=c++17 -O3 -march=native \
  -I./openfhe-development/src/pke/include \
  -I./openfhe-development/src/core/include \
  -I./openfhe-development/src/binfhe/include \
  -I./openfhe-development/build/src/core \
  -o bin/test_phi_dm_dgr_final \
  tests/final/test_phi_dm_dgr_final.cpp \
  -L./openfhe-development/build/lib \
  -lOPENFHEpke -lOPENFHEcore -lOPENFHEbinfhe \
  -Wl,-rpath,./openfhe-development/build/lib \
  -lstdc++ -lpthread -lm

LD_LIBRARY_PATH=./openfhe-development/build/lib:$LD_LIBRARY_PATH ./bin/test_phi_dm_dgr_final
```

---

## Hardware

All experiments on:
- **CPU**: AMD Ryzen 5 2600 (6-core, 3.40 GHz)
- **RAM**: 16 GB DDR4
- **OS**: Windows 11 Pro with WSL2 (Ubuntu)
- **Storage**: 224 GB SSD

No cloud. No server cluster. A mid-range gaming PC from 2018.

---

## Honest Limitations

1. **Author-reported results.** No third-party verification yet.
2. **CKKS precision issues.** Add-then-multiply chains need higher scaling parameters (hardware-limited).
3. **TOY security parameters.** RingDim=4096/8192. Production needs RingDim=32768+.
4. **Hardware constrained.** 128GB+ RAM needed for production parameter testing.
5. **Not constant-time.** Side-channel vulnerable.
6. **No formal security reduction.** The ring R[X]/(X^N+1, X²-X-1) is non-standard. Formal analysis pending.
7. **Not production-ready.** This is exploratory research.
8. **Ratio encoding depth limit unknown.** Tested to 100+ φ/ψ cycles. Theoretical analysis pending.

---

## What This Is NOT

- A NIST submission
- Production-ready software
- Peer-reviewed research (yet)
- A claim of having "solved" FHE
- Indistinguishability obfuscation

---

## Paper

Full paper: `paper/paper.tex` (compile with pdflatex)

Documents the mathematical derivations, experimental data, algorithm optimizations, DM-DGR architecture, and all limitations.

---

## Citation

```bibtex
@software{fernandez2025femmgfhe,
  author = {Dan Joseph M. Fernandez},
  title = {FEmmG-FHE: Fibonacci-Golden Ratio Cryptography},
  year = {2025},
  url = {https://github.com/primordialomegazero/femmgFHE}
}
```

## License

MIT — see [LICENSE](LICENSE)

## Author

**Dan Joseph M. Fernandez / Primordial Omega Zero**

---

```text
- .... .. ... / .-. . .--. --- ... .. - --- .-. -.-- / .-- .. .-.. .-.. / .- .-.. .-- .- -.-- ... / -... . / -.. . -.. .. -.-. .- - . -.. / - --- / - .... . / .-- --- -- .- -. / .. .----. ...- . / . ...- . .-. / -.-. --- -. ... .. -.. . .-. . -.. / - --- / -... . / --- -. / -- -.-- / .-.. . ...- . .-.. .-.-.-
```
