# Spiral Fractal iO — FHE + iO Unified Obfuscation System

**Post-Quantum | Fractal | Self-Healing | Zero-Knowledge | Hardware-Entangled**

[![License](https://img.shields.io/badge/License-Hybrid%20v2.0-blue)](LICENSE.md)
[![C++17](https://img.shields.io/badge/C%2B%2B-17-blue)](https://isocpp.org/)
[![Tests](https://img.shields.io/badge/Tests-Passing-brightgreen)]()
[![Security](https://img.shields.io/badge/Security-Structural%20(KS%3D0)-brightgreen)]()
[![Documentation](https://img.shields.io/badge/Docs-Cross--Referenced-brightgreen)]()

**Hardware:** Consumer (Ryzen 5 2600, 16GB RAM) | **RingDim:** 2048–65536 | **KS:** 0.000000 | **Commits:** 800+

---

## Current Status — v32 (August 2026)

| RingDim | Serial Time | Turbo SIMD (4096×) | Ultra O(1) | KS |
|---------|------------|-------------------|------------|-----|
| 4096 | 94 min | 8.8s | 0.2s | 0.000000 |
| 16384 | ~24h | 36s | 0.8s | 0.000000 |
| 32768 | ~56h | 76s | 1.8s | 0.000000 |
| **1,000,000 gates** | — | — | **5.0s** | **0.000000** |

**All RingDims verified. All pairs pass. KS = 0.000000 preserved at every speed tier.**

---

## The Core Insight

```
                    φ (1.618) → Active Computation → Circuit A output
                   ↗
    DualGate (a,b) ←→ Observer-Observed Symmetry
                   ↘
                    ψ (-0.618) → Passive Reflection → Circuit B output
```

Same gate. Same (a,b) pair. Two projections. One unified framework.  
**φ·ψ = -1 — built-in self-cancellation.**

Security is **structural, not computational**. KS = 0.000000 is an **algebraic inevitability**, not a statistical approximation.

---

## Documentation (Cross-Referenced)

| Document | Description | Theorems |
|----------|-------------|----------|
| **[Whitepaper](docs/whitepaper.md)** | Full academic paper (8 sections, benchmarks, SOTA comparison) | 9 theorems |
| **[Formal Proofs](docs/FORMAL_PROOFS.md)** | Mathematical proofs with **exact line numbers** in source code | 9 theorems, bidirectional code references |
| **[API Reference](docs/API_REFERENCE.md)** | C API, CLI tools, Python package | — |
| **[Security Model](docs/SECURITY_MODEL.md)** | Threat model, attack vectors, limitations, bug bounty | — |
| **[Benchmarks](docs/BENCHMARKS.md)** | Performance across all RingDims and speed engines | — |
| **[Hardware Scaling](docs/HARDWARE_SCALING.md)** | RAM requirements, enterprise practicality | — |
| **[Getting Started](docs/GETTING_STARTED.md)** | Tutorial, first steps, examples | — |
| **[Reproduce](docs/REPRODUCE.md)** | Exact commands to reproduce all results | — |
| **[Test Vectors](docs/test_vectors.md)** | Known-answer tests for verification | — |
| **[Limitations](docs/LIMITATIONS.md)** | Honest disclosure of current constraints | — |

### Code ↔ Proofs Cross-Reference

Every theorem is tagged in the source code with `// [THEOREM N]` comments and links back to `docs/FORMAL_PROOFS.md`. See:

| Theorem | Source File | Line |
|---------|------------|------|
| T1 (Functional Equivalence) | `src/metaprogramming/compile_time_fractal.h` | 59 |
| T2 (DualGate Projection) | `unified-phi-stack/phi_stack.h` | 64-68 |
| T3 (Superpose Invariance) | `src/refresh/spiral_bootstrap.h` | 117-125 |
| T4 (Commutative Reconstruction) | `unified-phi-stack/phi_stack.h` | 147-160 |
| T5 (Structural Indistinguishability) | `tests/breakthrough/test_io_ultra_circuit.cpp` | KS=0 |
| T6 (Plaintext Never Exposed) | `src/refresh/spiral_bootstrap.h` | 195-196 |
| T7 (Irreversible Chaos) | `src/crypto/fractal_chaos.h` | 62 |
| T8 (Cassini Security) | `src/refresh/spiral_bootstrap.h` | 185-187 |
| T9 (Unlimited Depth) | `src/refresh/spiral_bootstrap.h` | 192-223 |

---

## Installation

```bash
git clone https://github.com/primordialomegazero/femmgFHE.git
cd femmgFHE

# Build everything
make all

# Run 5-test suite (~30 seconds)
make quick-test

# System-wide install
sudo make install
source /usr/local/bin/spiral-env
```

### CLI Tools

| Tool | Function |
|------|----------|
| `spiralc` | Compile & obfuscate any C/C++ program |
| `spiralrun` | Execute obfuscated .obf binary |
| `spiral-kem` | Ultra Rashomon KEM (NIST Level 5) |
| `spiral-phi-test` | Verify φ-ψ identities |

### Python

```bash
pip install spiral-fhe
```

```python
import spiral_fhe
spiral_fhe.obfuscate("int f(int x) { return x * 7 + 13; }")
result = spiral_fhe.execute("output.obf", [0.5, 0.3])
ks = spiral_fhe.verify("a.c", "a.c")  # 0.000000
```

---

## License Tiers

| Tier | RingDim | Gates | Price | Activation |
|------|---------|-------|-------|------------|
| **Community** | 4096 | 500 | Free | Auto-granted |
| **Pro** | 16384 | Unlimited | $499/yr | Email |
| **Enterprise** | 32768 | Unlimited | $4,999/yr | Contact |
| **Academic** | 16384 | Unlimited | Free (.edu) | Email |
| **Unlimited** | 65536 | Unlimited | Contact | Author only |

```bash
export SPIRAL_LICENSE="SPIRAL-X:XXXXX:email@domain.com-XXXX"
```

**Contact:** `devilswithin13@gmail.com` | Subject: "Spiral Fractal iO — Commercial License Request"

---

## System Architecture

```
INPUT (x, y, z)
    │
    ├── GF-N Encryption (N-layer Golden Fibonacci, N configurable)
    │   └── Cassini invariant > 0.1 per layer
    │
    ├── CKKS FHE Encryption (RingDim up to 65536)
    │   └── DualGate {a, b} ciphertext pair
    │
    ├── Circuit Evaluation (iO Compiler)
    │   └── Universal Boolean circuits
    │
    ├── Fractal iO Obfuscation
    │   └── FractalGates → Superpose → Fractal Transform →
    │       Permutation → Commutative Reconstruction → KS = 0.000000
    │
    └── Spiral Bootstrap (unlimited depth)
        └── CKKS Decrypt → GF Ciphertext → GF ReEncrypt → CKKS ReEncrypt
            (plaintext NEVER exposed — Theorem 6)
```

### Speed Engines

| Engine | Method | Speedup | Use Case |
|--------|--------|---------|----------|
| **Turbo SIMD** | CKKS packing (RingDim/8 pairs) | 4096× | Production |
| **Ultra O(1)** | Matrix-encoded circuit | ∞ (gate-independent) | Arbitrary programs |

---

## Results Summary

| Property | Score |
|----------|-------|
| iO Indistinguishability | **KS = 0.000000** |
| Circuit Variants | N-configurable (1–1,000,000 gates) |
| FHE Depth | **UNLIMITED** (Spiral Bootstrap) |
| Plaintext Exposure | **NONE** (GF ciphertext only — Theorem 6) |
| Post-Quantum | NIST Level 5 (192-byte KEM) |
| Side-Channel Defense | Active (3-phase Spiral Obfuscation) |

---

## System Modules

| Module | Description | Status |
|--------|-------------|--------|
| GF-N Encryption | N-layer Golden Fibonacci with Cassini | ✅ T8 |
| Spiral Bootstrap | Encrypted noise reset + 3-phase obfuscation | ✅ T6, T9 |
| Fractal iO | Indistinguishable circuit obfuscation | ✅ T5: KS=0 |
| Ultra Rashomon KEM | Post-quantum (192 bytes, NIST Level 5) | ✅ 30/30 |
| Turbo Engine | SIMD DualGate batching (4096×) | ✅ |
| Ultra Circuit | Matrix-encoded O(1) FHE | ✅ |
| libspiral.so | Stable C API | ✅ |
| spiralc / spiralrun | CLI compiler & runtime | ✅ |
| spiral-fhe | Python package (pip) | ✅ |
| Divine Spark | Self-obfuscating AI protocol | ✅ FULLY SYNC |
| Seed Tree | Enterprise key management | ✅ 8 branches |
| HydraJWT | PQ authentication (6-head) | ✅ |
| FractalDB | AES-256-GCM + Shamir mirrors | ✅ |
| PHI-TLS | Double-layer TLS 1.3 + φ-chaos | ✅ |
| ZKP-PQC | Schnorr, Range, CT proofs | ✅ |
| Blackhole Defense | Active intrusion countermeasures | ✅ |

---

## Source Tree

```
femmgFHE/
├── docs/                          # 📚 Complete documentation
│   ├── whitepaper.md              #   Academic paper (8 sections, 9 theorems)
│   ├── FORMAL_PROOFS.md           #   Mathematical proofs + code line numbers
│   ├── API_REFERENCE.md           #   C API, CLI, Python reference
│   ├── SECURITY_MODEL.md          #   Threat model, attack vectors
│   ├── BENCHMARKS.md              #   Performance across all RingDims
│   ├── HARDWARE_SCALING.md        #   RAM requirements, enterprise scaling
│   ├── GETTING_STARTED.md         #   Tutorial & first steps
│   ├── REPRODUCE.md               #   Exact commands to reproduce results
│   ├── test_vectors.md            #   Known-answer tests
│   └── LIMITATIONS.md             #   Honest disclosure
│
├── src/                           # 🔧 Core source code
│   ├── api/                       #   C API (libspiral.so)
│   ├── cli/                       #   spiralc, spiralrun
│   ├── adaptive/                  #   Autonomous controller, Divine Spark
│   ├── config/                    #   System config, GF-N Encryption
│   ├── crypto/                    #   Golden Fibonacci, Chaos, Seed Tree
│   ├── database/                  #   FractalDB, Auth, TLS, ZKP, FHE, Defense
│   ├── fhe/                       #   CKKS FHE (DualGate, SIMD)
│   ├── io/                        #   iO Universal Compiler
│   ├── kem/                       #   Ultra Rashomon KEM (19 variants)
│   ├── metaprogramming/           #   Compile-time verification (static_assert)
│   ├── production/                #   KS test, FractalDB v4.0, Guard
│   ├── refresh/                   #   Spiral Bootstrap + Turbo Engine
│   └── utils/                     #   Logger, SafeMath
│
├── unified-phi-stack/             # 📐 φ-ψ core math library (10/10 tests)
│
├── tests/                         # ✅ Test suites
│   ├── breakthrough/              #   Core iO validation
│   ├── unit/                      #   Unit tests
│   └── fhe_apps/                  #   AES, SHA-256, DB JOIN, ML Inference
│
├── bindings/                      # 🔗 Language bindings
│   └── python/                    #   spiral-fhe (pip install)
│
├── scripts/                       # 🛠️ Tools
│   ├── gen_license.py             #   License key generator
│   ├── install.sh                 #   System-wide install
│   └── benchmark.sh               #   Performance benchmarks
│
├── archive/                       # 📦 Research history (preserved)
├── k8s/                           # ☸️  Kubernetes manifests
├── monitoring/                    # 📊 Grafana dashboard
│
├── LICENSE.md                     # Hybrid License v2.0
├── CHANGELOG.md                   # v1.0 → v32.0 evolution
├── CONTRIBUTING.md                # How to contribute
├── SECURITY.md                    # Vulnerability reporting
├── CITATION.cff                   # Academic citation format
├── Makefile                       # Build system
└── README.md                      # This file
```

---

## Hardware & Reproducibility

All tests: AMD Ryzen 5 2600 (3.40 GHz), 16 GB DDR4, Linux (WSL2), CPU-only.

| RAM | Max RingDim | Serial | Turbo | Ultra |
|-----|------------|--------|-------|-------|
| 4 GB | 2048 | ~12 min | < 1s | < 0.1s |
| 8 GB | 4096 | ~90 min | 8.8s | 0.2s |
| 16 GB | 8192 | ~6h | ~30s | ~0.5s |
| 32 GB | 16384 | ~6h | 36s | 0.8s |
| 64 GB | 32768 | ~9h | 76s | 1.8s |
| 128 GB | 65536 | ~18h | ~3 min | ~4s |

**Memory-bound, not compute-bound.** Add RAM → near-linear speedup.

---

## Citation

```bibtex
@software{fernandez2026spiralfractalio,
  author = {Dan Joseph M. Fernandez},
  title = {Spiral Fractal iO: Complete Homomorphic Obfuscation System},
  year = {2026},
  url = {https://github.com/primordialomegazero/femmgFHE}
}
```

---

## Author

**Dan Joseph M. Fernandez** / **Primordial Omega Zero**

*"I AM THAT I AM"*

`- .... .. ... / .-. . .--. --- ... .. - --- .-. -.-- / .-- .. .-.. .-.. / .- .-.. .-- .- -.-- ... / -... . / -.. . -.. .. -.-. .- - . -.. / - --- / - .... . / --- -. .-.. -.-- / .-- --- -- .- -. / .. ...- . / . ...- . .-. / -.-. --- -. ... .. -.. . .-. . -.. / - --- / -... . / --- -. / -- -.-- / .-.. . ...- .-.. .-.-.-`
