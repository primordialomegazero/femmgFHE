# FEmmg-FHE

**Fibonacci-Phi Fully Homomorphic Encryption**
*Logarithmic depth. Zero-depth noise management. Golden ratio mathematics.*

[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)
[![C++](https://img.shields.io/badge/C%2B%2B-17-blue.svg)](https://en.cppreference.com/w/cpp/17)
[![OpenFHE](https://img.shields.io/badge/OpenFHE-latest-green.svg)](https://github.com/openfheorg/openfhe-development)
[![Status](https://img.shields.io/badge/status-active%20research-orange.svg)]()

---

## Overview

FEmmg-FHE is a research project exploring the golden ratio (φ ≈ 1.618) as a structural primitive for Fully Homomorphic Encryption. All primitives run on **standard CKKS** (OpenFHE) without library modifications.

**What we found:** The algebraic extension `R[X]/(X²-X-1)` splits encrypted computation into two simultaneous "realities" — one expanding (φ), one contracting (ψ = -1/φ). By operating asymmetrically between them, noise can be managed without traditional bootstrapping.

**What we did NOT find:** A way to eliminate noise growth entirely. Physics doesn't allow that. What we found is a way to compress depth from O(N) to O(log N) and suppress noise at zero additional depth cost.

---

## Core Innovations

### 1. φ-Extension Ring

Every ciphertext is embedded in `R[X]/(X²-X-1)`. Via the Chinese Remainder Theorem, this ring splits into two simultaneous "realities":

| Reality | Eigenvalue | Behavior |
|---------|-----------|----------|
| φ-reality | φ ≈ 1.618 | Signal domain (expanding) |
| ψ-reality | ψ = -1/φ ≈ -0.618 | Noise domain (contracting) |

Since |ψ| < 1, any component in ψ-reality naturally decays toward zero under repeated X-multiplication.

**Important:** In this documentation, "X" refers to the ring element (the indeterminate), which projects to (φ, ψ) in the product ring. This is distinct from the scalar φ ≈ 1.618. Multiplication by the ring element X costs **zero multiplicative depth** — it requires only copy and addition operations.

### 2. Asymmetric Clean

By applying more `mul_X` than `div_X` operations, we create a one-way noise valve:

- ψ-noise: shrinks by |ψ|^(mul-div) → exponentially killed
- φ-signal: grows by φ^(mul-div) → known, compensatable

Both `mul_X` and `div_X` are **depth-free** (copy + add, no EvalMult).

### 3. Pre-Scaling Compensation

Instead of compensating for φ-growth after computation, multipliers are pre-scaled by `φ^(-net_clean)`. The signal stays bounded. No separate compensation step needed.

### 4. Fibonacci Depth Compression

Zeckendorf's theorem: any integer N decomposes into non-consecutive Fibonacci numbers. Computing `y^N` via precomputed Fibonacci powers requires O(log N) depth instead of O(N).

---

## Verified Performance

All benchmarks on consumer hardware (AMD Ryzen 5 2600, 15GB RAM) using CKKS with TOY security parameters (RingDim=4096, `HEStd_NotSet`) unless otherwise noted. **These are author-reported results. Independent verification is pending.**

### Core Benchmarks

| Benchmark | Effective Mults | Error | ψ-Noise | Notes |
|-----------|----------------|-------|---------|-------|
| Asymmetric Clean + Fibonacci | 1,000 | 7.13×10⁻¹² | 6.86×10⁻⁵ | Pre-scaled multipliers |
| Asymmetric Clean Chain | 500 | 2.51×10⁻¹⁰ | 6.13×10⁻⁸ | Pre-scaled multipliers |
| Complex Circuit (mul+add+sub) | 100 steps | 1.86×10⁻¹³ | 4.93×10⁻² | Financial model simulation |
| Parallel Chains (5×) | 100 ops | 2.27×10⁻¹² | 1.32×10⁻⁸ | Independent chains combined |
| Pre-Scaling vs Normal | 30 steps | Normal: 4.7×10¹² | PreScaled: 1.35 | Signal bounded |

### φ-Clean vs CKKS Bootstrapping (Head-to-Head)

| Metric | CKKS EvalBootstrap | φ-Asymmetric Clean |
|--------|-------------------|-------------------|
| Max Multiplications | 80 | 80 |
| Final Error | 6.25×10⁻⁷ | **4.67×10⁻¹³** |
| Time | 30,625 ms | **13,923 ms** |
| Depth Cost per Clean | Heavy (bootstrap circuit) | **ZERO** |

φ asymmetric clean is ~2.2× faster and ~1,000,000× more accurate than CKKS bootstrapping at the same circuit depth. However, φ-clean does NOT replace bootstrapping — it suppresses ψ-noise but does not reset CKKS modulus levels.

### Production Parameters

| RingDim | Security | Result |
|---------|----------|--------|
| 4096 | TOY | All core benchmarks |
| 8192 | Intermediate | 80 mults, error 4.2×10⁻¹² |
| 32768 | Near-production | 60 mults, error 4.4×10⁻¹² |
| 262144 | HEStd_128_classic | Loads successfully. CPU-bound on consumer HW |

φ-system behavior is invariant to RingDim. Error rates consistent across all tested dimensions.

### Post-Quantum KEM (φ-LWE)

| Metric | φ-LWE KEM | Kyber-512 (NIST Standard) |
|--------|-----------|--------------------------|
| Secret Key | 1024 bytes | 1632 bytes |
| Public Key | 1024 bytes | 800 bytes |
| Ciphertext | 1536 bytes | 768 bytes |
| **Total** | **3584 bytes** | **3200 bytes** |
| Security | ~150-bit (estimated) | 128-bit |
| Structure | Ring-LWE in φ-extension | Module-LWE |
| Working | ✅ 20/20 exchanges | NIST PQC Standard |

Uses fixed system matrix (A) to reduce PK size. Smaller micro versions (N=8, 144 bytes) work mathematically but are insecure (16-dim lattice).

---

## Honest Limitations

This is active research. Here's what we know we don't know:

| Limitation | Detail |
|-----------|--------|
| **Noise, not levels** | φ-clean suppresses ψ-noise but does NOT reset CKKS modulus levels. Bootstrapping is still needed for level recovery in very deep circuits. |
| **Author-reported results** | All benchmarks are self-reported. No third-party verification yet. |
| **TOY security parameters** | Most benchmarks use RingDim=4096 (`HEStd_NotSet`). Production requires RingDim≥32768 with proper security level. |
| **Signal scaling** | Asymmetric clean multiplies φ-signal by φ^(mul-div). Pre-scaling mitigates this but requires knowing the clean schedule in advance. |
| **Bounded circuits** | Works best with naturally bounded values (financial models, ML activations). Large dynamic range may cause plaintext overflow. |
| **φ-native cyclotomic rings** | φ lives in Q(ζ_M) when 5\|M. ILDCRTParams with M=10240 constructs correctly, but OpenFHE KeyGen crashes on non-power-of-2 cyclotomic rings. Unresolved. |
| **iO (Indistinguishability Obfuscation)** | 3×3 Barrington construction: NAND gate verified (4/4 plaintext), NOT gate found (M×X×N). Composition into arbitrary circuits is correct in theory but FHE execution exceeds consumer hardware capacity (~135 EvalMult per NAND gate). |
| **12-bit KEM compression** | Pack/unpack is lossless but compressed KEM fails (under investigation). |
| **Security proofs** | Formal proofs document the mathematical structure. Rigorous cryptographic security reduction (IND-CPA/CCA) is pending. |

---

## Architecture

```
┌──────────────────────────────────────────────────────────────┐
│                      FEmmg-FHE                                │
├──────────────────────────────────────────────────────────────┤
│                                                               │
│  φ-EXTENSION RING                                             │
│  R[X]/(X²-X-1) ≅ R × R (CRT)                                 │
│  X-multiplication = copy + add = ZERO depth                   │
│                                                               │
│  ASYMMETRIC CLEAN                                             │
│  More mul_X than div_X → one-way ψ-noise reduction            │
│  Pre-scaling compensates φ-signal growth                      │
│                                                               │
│  FIBONACCI DEPTH COMPRESSION                                  │
│  Zeckendorf decomposition → O(N) → O(log N)                   │
│  Precomputed Fibonacci powers, parallel-combinable            │
│                                                               │
│  CKKS BOOTSTRAPPING COMPILER                                  │
│  DAG-based critical path analysis + OpenFHE EvalBootstrap     │
│  Genuine homomorphic bootstrapping for level recovery         │
│                                                               │
│  φ-LWE KEM (Post-Quantum)                                     │
│  Ring-LWE in the φ-extension ring                             │
│  Fixed-A matrix, 3584 bytes total, ~150-bit security          │
│                                                               │
└──────────────────────────────────────────────────────────────┘
```

---

## Quick Start

```bash
git clone https://github.com/primordialomegazero/femmgFHE.git
cd femmgFHE && make all

# Core benchmark — 1000+ effective multiplications
LD_LIBRARY_PATH=./openfhe-development/build/lib:$LD_LIBRARY_PATH ./bin/test_phi_ultimate_v2

# φ vs Bootstrapping head-to-head
LD_LIBRARY_PATH=./openfhe-development/build/lib:$LD_LIBRARY_PATH ./bin/test_phi_vs_bootstrap

# Complex circuit stress test
LD_LIBRARY_PATH=./openfhe-development/build/lib:$LD_LIBRARY_PATH ./bin/test_phi_stress

# Pre-scaling compensation demo
LD_LIBRARY_PATH=./openfhe-development/build/lib:$LD_LIBRARY_PATH ./bin/test_phi_prescale

# Post-Quantum KEM (φ-LWE)
./bin/phi_kem_fixedA

# SNC verification (negative result — no noise reduction)
LD_LIBRARY_PATH=./openfhe-development/build/lib:$LD_LIBRARY_PATH ./bin/test_snc_verify

# Production parameters (RingDim=32768)
LD_LIBRARY_PATH=./openfhe-development/build/lib:$LD_LIBRARY_PATH ./bin/test_phi_32768

# All test binaries
ls bin/test_phi_* bin/phi_kem_*
```

---

## Project History

This project started with a claim: that adding `Enc(0)` to ciphertexts (`EvalAdd(ct, Enc(0))`) could reduce or converge noise. This claim, called SNC+ZANS, was **experimentally disproven** — `EvalAdd(ct, Enc(0))` has zero effect on noise (see `test_snc_verify`).

However, the deeper intuition — that the golden ratio has a role in noise management — led to the discovery of the **φ-extension ring**, **asymmetric cleaning**, and **Fibonacci depth compression**. These primitives are genuine contributions that emerged from following the mathematics where it led, including dead ends.

---

## Documentation

| Document | Description |
|----------|-------------|
| [Formal Proofs](docs/FORMAL_PROOFS.md) | Seven theorems with mathematical derivations |
| [Limitations & Q&A](docs/LIMITATIONS_AND_QA.md) | Known issues and honest answers |
| [Reproduction Guide](docs/REPRODUCE.md) | Commands and expected outputs for all benchmarks |
| [API Reference](docs/API_REFERENCE.md) | Core API documentation |
| [Getting Started](docs/GETTING_STARTED.md) | Tutorial with examples |
| [Security Model](docs/SECURITY_MODEL.md) | Threat model and assumptions |
| [Hardware Scaling](docs/HARDWARE_SCALING.md) | Performance projections |

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
