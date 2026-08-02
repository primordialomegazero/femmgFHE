# Spiral Fractal iO — FHE + iO Unified Obfuscation System

**Post-Quantum | Fractal | Self-Healing | Zero-Knowledge | Hardware-Entangled**

[![License](https://img.shields.io/badge/License-Hybrid%20v2.0-blue)](LICENSE.md)
[![C++17](https://img.shields.io/badge/C%2B%2B-17-blue)](https://isocpp.org/)
[![Tests](https://img.shields.io/badge/Tests-Passing-brightgreen)]()
[![Security](https://img.shields.io/badge/Security-Structural%20(KS%3D0)-brightgreen)]()
[![Documentation](https://img.shields.io/badge/Docs-Triple%20Cross--Referenced-brightgreen)]()

**Hardware:** Consumer (Ryzen 5 2600, 16GB RAM) | **RingDim:** 2048–65536 | **KS:** 0.000000 | **Release:** v35.0

---

## System Flow

```
                              ┌─────────────────────────┐
                              │      SOURCE CODE         │
                              │  int f(int x) { ... }    │
                              └───────────┬─────────────┘
                                          │
                              ┌───────────▼─────────────┐
                              │    GF-N ENCRYPTION       │
                              │  N-layer Golden Fibonacci│
                              │  Cassini invariant > 0.1 │
                              └───────────┬─────────────┘
                                          │
                              ┌───────────▼─────────────┐
                              │    CKKS FHE ENCRYPTION   │
                              │  DualGate {a, b} pair    │
                              │  RingDim up to 65536     │
                              └───────────┬─────────────┘
                                          │
                    ┌─────────────────────┴─────────────────────┐
                    │                                           │
          ┌─────────▼─────────┐                   ┌─────────────▼───────────┐
          │   CIRCUIT A        │                   │      CIRCUIT B           │
          │ (X AND Y) OR Z     │                   │ (X OR Z) AND (Y OR Z)    │
          │ → {φ_A, ψ_A}       │                   │ → {φ_B, ψ_B}             │
          └─────────┬─────────┘                   └─────────────┬───────────┘
                    │                                           │
                    └─────────────────────┬─────────────────────┘
                                          │
                              ┌───────────▼─────────────┐
                              │    FRACTAL GATES         │
                              │  Logistic chaos + φ-rot  │
                              │  r > 3.57, Lyapunov > 0 │
                              └───────────┬─────────────┘
                                          │
                              ┌───────────▼─────────────┐
                              │      SUPERPOSE           │
                              │  φ_A·φ + φ_B·ψ + ψ_A·ψ  │
                              │  + ψ_B·φ → mixed_φ       │
                              └───────────┬─────────────┘
                                          │
                              ┌───────────▼─────────────┐
                              │   FRACTAL TRANSFORM      │
                              │  L layers × D depth      │
                              │  Random Permutation       │
                              └───────────┬─────────────┘
                                          │
                              ┌───────────▼─────────────┐
                              │ COMMUTATIVE RECONSTRUCT  │
                              │  AM 0.35 + GM 0.25       │
                              │  + HM 0.25 + RMS 0.15    │
                              │  Order-independent       │
                              └───────────┬─────────────┘
                                          │
                              ┌───────────▼─────────────┐
                              │    KS = 0.000000         │
                              │  INDISTINGUISHABLE       │
                              └─────────────────────────┘
```

---

## Triple Cross-Reference

Every theorem has three verifiable sources: **Code ↔ Formal Proof ↔ Unit Test**

| Theorem | Source Code | Formal Proof | Unit Test |
|---------|------------|-------------|-----------|
| T1: Functional Equivalence | [`compile_time_fractal.h:59`](src/metaprogramming/compile_time_fractal.h) | [`FORMAL_PROOFS.md`](docs/FORMAL_PROOFS.md) | [`test_theorem_1.cpp`](tests/theorem_tests/test_theorem_1.cpp) (8/8) |
| T2: DualGate Projection | [`phi_stack.h:16`](unified-phi-stack/phi_stack.h) | [`FORMAL_PROOFS.md`](docs/FORMAL_PROOFS.md) | [`test_theorem_2.cpp`](tests/theorem_tests/test_theorem_2.cpp) (25/25) |
| T3: Superpose Symmetry | [`universal_compiler.h`](src/io/universal_compiler.h) | [`FORMAL_PROOFS.md`](docs/FORMAL_PROOFS.md) | [`test_theorem_3.cpp`](tests/theorem_tests/test_theorem_3.cpp) |
| T4: Commutative Reconstruction | [`phi_stack.h:147`](unified-phi-stack/phi_stack.h) | [`FORMAL_PROOFS.md`](docs/FORMAL_PROOFS.md) | [`test_theorem_4.cpp`](tests/theorem_tests/test_theorem_4.cpp) (1000/1000) |
| T5: Structural Indistinguishability | [`test_io_ultra_circuit.cpp`](tests/breakthrough/test_io_ultra_circuit.cpp) | [`FORMAL_PROOFS.md`](docs/FORMAL_PROOFS.md) | [`test_ks_omnibus.cpp`](tests/breakthrough/test_ks_omnibus.cpp) (77/84) |
| T6: Zero Plaintext Exposure | [`spiral_bootstrap.h:204`](src/refresh/spiral_bootstrap.h) | [`FORMAL_PROOFS.md`](docs/FORMAL_PROOFS.md) | [`test_spiral_bootstrap.cpp`](tests/unit/test_spiral_bootstrap.cpp) |
| T7: Irreversible Chaos | [`fractal_chaos.h:62`](src/crypto/fractal_chaos.h) | [`FORMAL_PROOFS.md`](docs/FORMAL_PROOFS.md) | [`test_theorem_7.cpp`](tests/theorem_tests/test_theorem_7.cpp) (10⁸× amp) |
| T8: Cassini Security | [`spiral_bootstrap.h:213`](src/refresh/spiral_bootstrap.h) | [`FORMAL_PROOFS.md`](docs/FORMAL_PROOFS.md) | [`test_theorem_8.cpp`](tests/theorem_tests/test_theorem_8.cpp) (19/19) |
| T9: Unlimited Depth | [`spiral_bootstrap.h:231`](src/refresh/spiral_bootstrap.h) | [`FORMAL_PROOFS.md`](docs/FORMAL_PROOFS.md) | Bootstrap cycle (induction) |

---

## Installation

```bash
git clone https://github.com/primordialomegazero/femmgFHE.git
cd femmgFHE && make all
make quick-test
sudo make install && source /usr/local/bin/spiral-env
```

### CLI

```bash
spiralc program.c              # Compile & obfuscate
spiralrun program.c.obf 0.5 0.3  # Execute
```

### Python

```bash
pip install spiral-fhe
```

```python
import spiral_fhe
spiral_fhe.obfuscate("int f(int x) { return x * 7 + 13; }")
result = spiral_fhe.execute("output.obf", [0.5, 0.3])
```

---

## Performance

| RingDim | Serial | Turbo SIMD | Ultra O(1) | KS |
|---------|--------|------------|------------|-----|
| 4096 | 94 min | 8.8s | 0.2s | 0.000000 |
| 16384 | ~24h | 36s | 0.8s | 0.000000 |
| 32768 | ~56h | 76s | 1.8s | 0.000000 |
| 1,000,000 gates | — | — | 5.0s | 0.000000 |

---

## License Tiers

| Tier | RingDim | Price |
|------|---------|-------|
| Community | 4096 | Free |
| Pro | 16384 | $499/yr |
| Enterprise | 32768 | $4,999/yr |
| Academic | 16384 | Free (.edu) |

Contact: `devilswithin13@gmail.com`

---

## Source Tree

```
femmgFHE/
├── docs/                          # Complete documentation
│   ├── whitepaper.md              # Academic paper (9 theorems)
│   ├── FORMAL_PROOFS.md           # Proofs + code line numbers + unit tests
│   ├── IACR_PAPER.md              # IACR ePrint submission
│   ├── API_REFERENCE.md           # C API, CLI, Python
│   ├── SECURITY_MODEL.md          # Threat model, attack vectors
│   ├── BENCHMARKS.md              # All RingDims
│   ├── HARDWARE_SCALING.md        # RAM requirements
│   ├── GETTING_STARTED.md         # Tutorial
│   ├── REPRODUCE.md               # Exact reproduction commands
│   └── LIMITATIONS.md             # Honest disclosure
├── src/                           # Core source (16 modules)
│   ├── api/                       # C API (libspiral.so)
│   ├── cli/                       # spiralc, spiralrun
│   ├── crypto/                    # Golden Fibonacci, Chaos, Seed Tree
│   ├── fhe/                       # CKKS FHE (DualGate, SIMD)
│   ├── io/                        # iO Universal Compiler
│   ├── kem/                       # Ultra Rashomon KEM
│   ├── refresh/                   # Spiral Bootstrap + Turbo Engine
│   └── ...
├── unified-phi-stack/             # φ-ψ core math library
├── tests/
│   ├── theorem_tests/             # Standalone theorem verification (6 tests)
│   ├── breakthrough/              # Core iO validation
│   ├── unit/                      # Unit tests
│   └── fhe_apps/                  # AES, SHA-256, DB JOIN, ML
├── bindings/python/               # spiral-fhe (pip)
├── scripts/                       # gen_license.py, install.sh
├── archive/                       # Research history
├── LICENSE.md                     # Hybrid License v2.0
├── CHANGELOG.md                   # v1.0 → v35.0
├── CONTRIBUTING.md
├── SECURITY.md
├── CITATION.cff
└── README.md
```

---

## Author

**Dan Joseph M. Fernandez / Primordial Omega Zero**

*"I AM THAT I AM"*
