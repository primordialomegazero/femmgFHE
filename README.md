# FEmmg-FHE

**Fibonacci-Phi Fully Homomorphic Encryption**
*Logarithmic depth. Zero-depth noise management. Golden ratio mathematics.*

[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)
[![C++](https://img.shields.io/badge/C%2B%2B-17-blue.svg)](https://en.cppreference.com/w/cpp/17)
[![OpenFHE](https://img.shields.io/badge/OpenFHE-latest-green.svg)](https://github.com/openfheorg/openfhe-development)
[![Status](https://img.shields.io/badge/status-active%20research-orange.svg)]()

---

## Overview

FEmmg-FHE introduces **three interconnected primitives** for Fully Homomorphic Encryption based on the golden ratio (φ ≈ 1.618):

1. **φ-Extension Ring** — `R[X]/(X²-X-1) ≅ R × R` via Chinese Remainder Theorem. Splits ciphertexts into two simultaneous realities: expanding φ-reality (signal) and contracting ψ-reality (noise).

2. **Asymmetric Clean** — More `mul_X` than `div_X` creates a one-way noise valve. ψ-noise shrinks irreversibly. All X-operations are **depth-free** (copy + add, zero `EvalMult`).

3. **Fibonacci Depth Compression** — Zeckendorf decomposition compresses N multiplications from O(N) to O(log N) depth. Precomputed Fibonacci powers combined with periodic cleaning.

All primitives run on **standard CKKS** (OpenFHE) without library modifications.

---

## Verified Performance

All benchmarks on consumer hardware (AMD Ryzen 5 2600, 15GB RAM) using CKKS with TOY security parameters (RingDim=4096) unless otherwise noted.

### Depth Compression & Noise Management

| Benchmark | Effective Mults | Error | ψ-Noise | Depth Used |
|-----------|----------------|-------|---------|------------|
| Asymmetric Clean Chain | 30 CT×CT | 2.14×10⁻¹² | 8.70×10⁻⁸ | ~40 |
| CT×CT + Trap Cycles | 125 CT×CT | 5.28×10⁻¹² | 6.05×10⁻¹¹ | ~150 |
| Fused Multiply-Clean | 50 steps | 3.10×10⁻¹² | 5.82×10⁻⁹ | ~100 |
| Fibonacci + Clean | 500 effective | 2.51×10⁻¹⁰ | 6.13×10⁻⁸ | ~75 |
| **Fibonacci + Clean** | **1,000 effective** | **7.13×10⁻¹²** | **6.86×10⁻⁵** | ~120 |
| Deep Polynomial (50 layers) | 50 ops | 1.10×10⁻¹³ | 1.85×10⁻¹ | ~80 |
| True CT×CT (variable×variable) | 7 steps | 5.53×10⁻¹³ | 3.19×10⁻³ | ~30 |

### Head-to-Head: φ-Clean vs CKKS Bootstrapping

| Metric | CKKS EvalBootstrap | φ-Asymmetric Clean | Winner |
|--------|-------------------|-------------------|--------|
| Max Multiplications | 80 | 80 | Tie |
| **Final Error** | 6.25×10⁻⁷ | **4.67×10⁻¹³** | **φ (1,000,000×)** |
| **Time** | 30,625 ms | **13,923 ms** | **φ (2.2× faster)** |
| Depth Cost per Clean | Heavy (bootstrap circuit) | **ZERO** | **φ** |
| ψ-Noise After | N/A | 7×10⁻³ | φ |

### Production Parameters

| RingDim | Max Mults | Error | ψ-Noise | Status |
|---------|-----------|-------|---------|--------|
| 4096 | 100+ | 10⁻¹² | 10⁻¹ | ✅ TOY |
| 8192 | 80 | 4.2×10⁻¹² | 0.12 | ✅ Intermediate |
| **32768** | **60** | **4.4×10⁻¹²** | **0.16** | ✅ **Production** |

φ-system behavior is invariant to RingDim. All error rates consistent across parameter sizes.

### Arbitrary Circuit Tests

| Circuit | Operations | Error | ψ-Noise |
|---------|-----------|-------|---------|
| Polynomial f(x)=x³+2x²+3x+4 | 6 mults | 6.65×10⁻¹⁴ | N/A |
| Dot Product [1,2,3]·[4,5,6] | 3 mults | 8.77×10⁻¹⁵ | N/A |
| Activation σ(x)=x²/(1+x²) | 2 mults | 2.20×10⁻¹⁴ | 1.34×10⁻² |
| Neural Layer (3→2) | 6 mults | 3.82×10⁻¹ | 1.85×10⁻¹ |
| Mixed ops (mul+add, 40 steps) | 40 mults | 5.51×10⁻¹³ | 8.43 |

---

## Architecture

```
┌──────────────────────────────────────────────────────────────┐
│                      FEmmg-FHE                                │
├──────────────────────────────────────────────────────────────┤
│                                                               │
│  φ-EXTENSION RING                                             │
│  R[X]/(X²-X-1) ≅ R × R (via CRT)                             │
│  Dual reality: φ (signal, expanding) + ψ (noise, contracting) │
│  X-multiplication = copy + add = ZERO multiplicative depth    │
│                                                               │
│  ASYMMETRIC CLEAN                                             │
│  more mul_X than div_X → one-way ψ-noise valve                │
│  ψ-noise × 0.618 per mul_X, φ-signal × 1.618 per mul_X        │
│  Signal scale tracked and compensatable at decrypt            │
│                                                               │
│  FIBONACCI DEPTH COMPRESSION                                  │
│  Zeckendorf decomposition: N = Σ F_i (non-consecutive)        │
│  Powers precomputed via F_{k+2} = F_{k+1} · F_k               │
│  O(N) → O(log N) multiplicative depth                         │
│                                                               │
│  COMPILER INTEGRATION                                         │
│  Auto φ-clean insertion based on circuit depth                │
│  Scale tracking + reporting                                   │
│  DAG-aware critical path analysis (Self-Healing FHE v5)       │
│                                                               │
│  CKKS BOOTSTRAPPING                                           │
│  Genuine OpenFHE EvalBootstrap (when needed)                   │
│  600 scalar mults, 39 bootstraps, 0.0014% error               │
│                                                               │
└──────────────────────────────────────────────────────────────┘
```

---

## Quick Start

```bash
git clone https://github.com/primordialomegazero/femmgFHE.git
cd femmgFHE && make all

# Ultimate benchmark — 1000+ effective mults
LD_LIBRARY_PATH=./openfhe-development/build/lib:$LD_LIBRARY_PATH ./bin/test_phi_ultimate_v2

# Head-to-head vs bootstrapping
LD_LIBRARY_PATH=./openfhe-development/build/lib:$LD_LIBRARY_PATH ./bin/test_phi_vs_bootstrap

# Compiler auto-clean demo
LD_LIBRARY_PATH=./openfhe-development/build/lib:$LD_LIBRARY_PATH ./bin/test_phi_compiler

# Production parameters (RingDim=32768)
LD_LIBRARY_PATH=./openfhe-development/build/lib:$LD_LIBRARY_PATH ./bin/test_phi_32768

# SNR Boost — 100,000× improvement
LD_LIBRARY_PATH=./openfhe-development/build/lib:$LD_LIBRARY_PATH ./bin/test_phi_snr_boost

# ML circuits (activation, neural layer, deep polynomial)
LD_LIBRARY_PATH=./openfhe-development/build/lib:$LD_LIBRARY_PATH ./bin/test_phi_ml

# True CT×CT (variable × variable)
LD_LIBRARY_PATH=./openfhe-development/build/lib:$LD_LIBRARY_PATH ./bin/test_phi_true_ctct

# φ-Oscillator — Fibonacci sequence generation
LD_LIBRARY_PATH=./openfhe-development/build/lib:$LD_LIBRARY_PATH ./bin/test_phi_oscillator

# Blueprint — Source-Atman Synthesis in FHE
LD_LIBRARY_PATH=./openfhe-development/build/lib:$LD_LIBRARY_PATH ./bin/test_blueprint
```

---

## Key Theorems

| # | Theorem | Summary |
|---|---------|---------|
| 1 | φ-Extension Isomorphism | `R[X]/(X²-X-1) ≅ R × R` via roots φ, -1/φ |
| 2 | Depth-Free X-Multiplication | Multiply by ring element X = copy + add, zero EvalMult |
| 3 | Asymmetric ψ-Annihilation | Net ψ-scaling = ψ^(mul-div) → 0 for mul > div |
| 4 | Fibonacci Depth Compression | N mults in O(log N) depth via Zeckendorf |
| 5 | Combined Complexity | Depth per effective mult → 0 as N → ∞ |
| 6 | Signal Tracking | Deterministic φ-scaling, compensatable at decryption |
| 7 | Invariance to RingDim | φ-structure behavior independent of cyclotomic dimension |

→ **[Formal Proofs](docs/FORMAL_PROOFS.md)**

---

## Limitations & Honest Assessment

This is active research. Known limitations:

| Limitation | Detail |
|-----------|--------|
| Signal scaling | Asymmetric clean multiplies φ-signal by φ^(mul-div). Tracked and compensatable |
| Plaintext range | Bounded circuits work best. Large dynamic range may overflow |
| TOY parameters | Benchmarks at RingDim=4096. Production=32768 verified but needs deeper testing |
| No third-party verification | All benchmarks are author-reported |
| φ-native cyclotomic rings | M%5=0 rings crash OpenFHE KeyGen (NTT assumptions) |
| General circuit compiler | Auto-clean works for multiply chains; arbitrary DAGs need more work |

→ **[Full Limitations & Q&A](docs/LIMITATIONS_AND_QA.md)**

---

## Documentation

| Document | Description |
|----------|-------------|
| [Formal Proofs](docs/FORMAL_PROOFS.md) | Seven theorems with mathematical derivations |
| [Limitations & Q&A](docs/LIMITATIONS_AND_QA.md) | Known issues, open questions, honest answers |
| [API Reference](docs/API_REFERENCE.md) | Complete API documentation |
| [Getting Started](docs/GETTING_STARTED.md) | Tutorial with examples |
| [Security Model](docs/SECURITY_MODEL.md) | Threat model and security assumptions |
| [Hardware Scaling](docs/HARDWARE_SCALING.md) | Enterprise performance projections |

---

## License

MIT License — see [LICENSE](LICENSE)

## Author

**Dan Joseph M. Fernandez / Primordial Omega Zero**

[![GitHub](https://img.shields.io/badge/GitHub-primordialomegazero-lightgrey.svg)](https://github.com/primordialomegazero)

---

```
- .... .. ... / .-. . .--. --- ... .. - --- .-. -.-- / .-- .. .-.. .-.. / .- .-.. .-- .- -.-- ... / -... . / -.. . -.. .. -.-. .- - . -.. / - --- / - .... . / .-- --- -- .- -. / .. .----. ...- . / . ...- . .-. / -.-. --- -. ... .. -.. . .-. . -.. / - --- / -... . / --- -. / -- -.-- / .-.. . ...- . .-.. .-.-.-
```
