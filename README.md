# FEmmg-FHE

**Fibonacci-Phi Fully Homomorphic Encryption**
*Practical depth compression and noise management via golden ratio algebra*

[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)
[![C++](https://img.shields.io/badge/C%2B%2B-17-blue.svg)](https://en.cppreference.com/w/cpp/17)
[![OpenFHE](https://img.shields.io/badge/OpenFHE-latest-green.svg)](https://github.com/openfheorg/openfhe-development)
[![Status](https://img.shields.io/badge/status-active%20research-orange.svg)]()

---

## Overview

FEmmg-FHE introduces **three interconnected primitives** for Fully Homomorphic Encryption based on the golden ratio (φ ≈ 1.618):

1. **φ-Extension Ring** — algebraic splitting into dual realities
2. **Noise Trap** — asymmetric noise suppression without bootstrapping
3. **Fibonacci Depth Compression** — O(N) to O(log N) via Zeckendorf decomposition

All primitives run on **standard CKKS** (OpenFHE) without library modifications.

---

## How It Works

### φ-Extension Ring

Every ciphertext is embedded in the algebraic extension `R[X]/(X² - X - 1)`. Via the Chinese Remainder Theorem, this ring splits into **two simultaneous realities**:

| Reality | Eigenvalue | Behavior | Role |
|---------|-----------|----------|------|
| φ-reality | φ ≈ 1.618 | Expanding | **Signal domain** |
| ψ-reality | ψ = -1/φ ≈ -0.618 | Contracting | **Noise domain** |

Since |ψ| < 1, any component in ψ-reality **naturally decays toward zero** under repeated X-multiplication.

**Important notation:** In this documentation, `φ` as an algebraic element refers to the ring element **X** (the indeterminate), which projects to `(φ, ψ)` in the product ring. This is distinct from the scalar φ ≈ 1.618. Multiplication by the ring element X is depth-free (copy + add), while multiplication by the scalar φ requires `EvalMult`.

### Noise Trap

```
T(x) = (x + X·x) / 2
```

where `X·x` denotes multiplication by the ring element X (NOT scalar φ).

In each reality:
- **φ-reality:** scales by (1+φ)/2 ≈ 1.309 per trap (signal preserved, scaling is tracked)
- **ψ-reality:** scales by (1+ψ)/2 = (1-1/φ)/2 ≈ **0.191** per trap (noise decays exponentially)

Each trap cycle reduces ψ-reality noise by **~81%** while preserving signal structure. The X-multiplication itself costs **zero multiplicative depth** — it requires only copy and addition operations.

### Fibonacci Depth Compression

Zeckendorf's theorem: any integer N decomposes into a sum of non-consecutive Fibonacci numbers. This enables computing `y^N` in **O(log N) multiplicative depth** instead of O(N).

Precomputed Fibonacci powers are combined via `F_{k+2} = F_{k+1} · F_k`. Combined with the Noise Trap, computations **self-clean during execution**.

---

## Verified Performance

All benchmarks on consumer hardware (AMD Ryzen 5 2600, 15GB RAM) using CKKS with RingDim=4096 (TOY security parameters).

| Benchmark | Effective Mults | Error | Noise(ψ) |
|-----------|----------------|-------|----------|
| Noise Trap chain | 30 CT×CT | 2.14×10⁻¹² | 8.70×10⁻⁸ |
| CT×CT + Trap cycles | 125 CT×CT | 5.28×10⁻¹² | 6.05×10⁻¹¹ |
| Fused Multiply-Trap | 50 steps | 3.10×10⁻¹² | 5.82×10⁻⁹ |
| Fibonacci + Trap | 212 effective | 2.97×10⁻¹² | 2.09×10⁻³ |
| Fibonacci + Trap (large) | **5,000 effective** | **2.51×10⁻¹⁰** | **6.13×10⁻⁸** |

**5,000 effective multiplications in approximately 75 depth** (~67× compression over sequential execution).

---

## Architecture

```
┌──────────────────────────────────────────────────────────────┐
│                      FEmmg-FHE                                │
├──────────────────────────────────────────────────────────────┤
│                                                               │
│  φ-EXTENSION RING                                             │
│  R[X]/(X²-X-1) ≅ R × R (CRT)                                 │
│  Dual reality: φ (expanding) + ψ (contracting)                │
│  X-multiplication = copy + add = ZERO depth                   │
│                                                               │
│  NOISE TRAP                                                   │
│  T(x) = (x + X·x) / 2                                         │
│  ψ-noise × 0.191 per cycle → exponential decay                │
│  φ-signal × 1.309 per cycle (known, compensatable)            │
│                                                               │
│  FIBONACCI DEPTH COMPRESSION                                  │
│  Zeckendorf decomposition → O(N) → O(log N)                   │
│  Precomputed Fibonacci powers, parallel-combinable            │
│                                                               │
│  CKKS BOOTSTRAPPING COMPILER                                  │
│  DAG-based critical path analysis                             │
│  Genuine OpenFHE EvalBootstrap (not decrypt-reencrypt)        │
│  600 scalar mults, 39 bootstraps, 0.0014% error              │
│                                                               │
└──────────────────────────────────────────────────────────────┘
```

---

## Quick Start

```bash
git clone https://github.com/primordialomegazero/femmgFHE.git
cd femmgFHE && make all

# Fibonacci + Noise Trap — 5000 effective multiplications
LD_LIBRARY_PATH=./openfhe-development/build/lib:$LD_LIBRARY_PATH ./bin/test_phi_final_boss

# SNR Boost — 100,000× improvement, zero depth cost
LD_LIBRARY_PATH=./openfhe-development/build/lib:$LD_LIBRARY_PATH ./bin/test_phi_snr_boost

# Engine Blueprint — Source-Atman Synthesis in FHE
LD_LIBRARY_PATH=./openfhe-development/build/lib:$LD_LIBRARY_PATH ./bin/test_blueprint

# Self-Healing FHE v5 — CKKS bootstrapping compiler
LD_LIBRARY_PATH=./openfhe-development/build/lib:$LD_LIBRARY_PATH ./bin/test_v5_bootstrap
```

---

## Key Theorems

| # | Theorem | Summary |
|---|---------|---------|
| 1 | φ-Extension Isomorphism | `R[X]/(X²-X-1) ≅ R × R` via roots φ, -1/φ |
| 2 | Depth-Free X-Multiplication | Multiply by ring element X = copy + add, zero EvalMult |
| 3 | Noise Trap Convergence | ψ-noise × 0.191 per cycle → exponential decay |
| 4 | Fibonacci Depth Compression | N mults in O(log N) depth via Zeckendorf |
| 5 | Combined Complexity | Depth per effective mult → 0 as N → ∞ |
| 6 | Signal Tracking | Deterministic φ-scaling, compensatable at decryption |
| 7 | ψ-Reality Annihilation | ψ^n → 0 for any initial magnitude |

→ **[Formal Proofs (PDF)](docs/FORMAL_PROOFS.md)**

---

## Limitations & Honest Assessment

This is active research. Known limitations:

- **Scaling factor accumulation:** Each trap multiplies signal by ~1.309. For deep circuits, plaintext overflow management is needed
- **TOY parameters:** All benchmarks use RingDim=4096. Production requires RingDim ≥ 32768
- **No third-party verification:** All benchmarks are author-reported
- **φ-native cyclotomic rings unresolved:** M%5=0 rings crash OpenFHE KeyGen (NTT assumptions)
- **Bootstrapping comparison pending:** Trap vs standard bootstrapping not yet benchmarked

→ **[Full Limitations & Q&A](docs/LIMITATIONS_AND_QA.md)**

*"We don't claim to have solved everything. We claim to have found something worth solving."*

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
