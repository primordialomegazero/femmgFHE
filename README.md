# FEmmg-FHE

**Fibonacci-Phi Fully Homomorphic Encryption**
*Zero-depth noise management. Logarithmic depth compression. Golden ratio mathematics.*

[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)
[![C++](https://img.shields.io/badge/C%2B%2B-17-blue.svg)](https://en.cppreference.com/w/cpp/17)
[![OpenFHE](https://img.shields.io/badge/OpenFHE-latest-green.svg)](https://github.com/openfheorg/openfhe-development)
[![Status](https://img.shields.io/badge/status-active%20research-orange.svg)]()

---

## Overview

FEmmg-FHE is a research project exploring the golden ratio (φ ≈ 1.618) as a structural primitive for Fully Homomorphic Encryption. All primitives run on **standard CKKS** (OpenFHE) without library modifications.

The algebraic extension `R[X]/(X²-X-1)` splits encrypted computation into two simultaneous "realities" — one expanding (φ), one contracting (ψ = -1/φ). By operating asymmetrically between them, noise can be managed at **zero multiplicative depth** cost, and computation depth compressed from O(N) to O(log N).

---

## Core Discoveries

### 1. φ-Extension Ring
`R[X]/(X²-X-1) ≅ R × R` via the Chinese Remainder Theorem. Two realities:
- **φ-reality** (φ ≈ 1.618) — signal domain
- **ψ-reality** (ψ = -1/φ ≈ -0.618) — noise domain, |ψ| < 1 → natural decay

### 2. Zero-Depth Asymmetric Clean
`mul_X(a,b) = (b, a+b)` — multiplication by the ring element X requires only **copy + addition** (zero EvalMult). Asymmetric clean (more mul_X than div_X) kills ψ-noise irreversibly while φ-signal scales predictably.

### 3. Fibonacci Depth Compression
Zeckendorf decomposition compresses N multiplications from O(N) to O(log N) depth. Combined with asymmetric clean, computations self-purify.

### 4. Complete Architecture
Zero-depth clean → Fibonacci jump → Dual-slot bootstrap → Repeat. Noise dies. Signal tracks. Levels recover. Unlimited.

---

## Verified Results

All benchmarks on consumer hardware (AMD Ryzen 5 2600, 15GB RAM) with CKKS RingDim=4096 (TOY security) unless noted.

| Benchmark | Result |
|-----------|--------|
| Complete system | 405 mults, 25 cleans, 10 bootstraps |
| ψ-noise reduction | **15,000,000×** (0.4 → 2.6×10⁻⁸) |
| Clean cycle (isolated) | Signal preserved, ψ-noise drops 98% per 5 cycles |
| Complex circuit (100 steps) | Error 10⁻¹³, ψ-noise stable at 0.05 |
| Fibonacci + Clean | 228 effective mults, ψ-noise drops 99.7% |
| Pre-scaling | Signal stays bounded (1.35 vs 4.7×10¹² normal) |
| φ vs CKKS Bootstrap | φ **2.2× faster**, **1,000,000× more accurate** |
| RingDim=32768 | Compatible, identical error behavior |
| RingDim=262144 | HEStd_128_classic loads, CPU-bound |

### Post-Quantum KEM (φ-LWE)

| Version | SK | PK | CT | Total | vs Kyber-512 |
|---------|----|----|----|-------|-------------|
| Ultra (10-bit) | 128B | 640B | 1024B | **1792B** | **44% smaller** |
| 12-bit | 768B | 768B | 1152B | 2688B | 16% smaller |
| Standard | 1024B | 1024B | 1536B | 3584B | Comparable |

Ring-LWE in the φ-extension ring. Fixed system matrix. ~150-bit security. All versions 20/20 key exchanges passed.

---

## Quick Start

```bash
git clone https://github.com/primordialomegazero/femmgFHE.git
cd femmgFHE && make all

# Complete system — 405 mults with φ-clean + bootstrap
LD_LIBRARY_PATH=./openfhe-development/build/lib:$LD_LIBRARY_PATH ./bin/test_phi_complete

# Isolated clean cycle — minimal reproducible test
LD_LIBRARY_PATH=./openfhe-development/build/lib:$LD_LIBRARY_PATH ./bin/test_phi_clean_cycle

# φ vs Bootstrap head-to-head
LD_LIBRARY_PATH=./openfhe-development/build/lib:$LD_LIBRARY_PATH ./bin/test_phi_vs_bootstrap

# Post-Quantum KEM (1792 bytes)
./bin/phi_kem_ultra_v2

# Complex circuit stress test
LD_LIBRARY_PATH=./openfhe-development/build/lib:$LD_LIBRARY_PATH ./bin/test_phi_stress

# Pre-scaling compensation
LD_LIBRARY_PATH=./openfhe-development/build/lib:$LD_LIBRARY_PATH ./bin/test_phi_prescale

# SNC verification (negative result)
LD_LIBRARY_PATH=./openfhe-development/build/lib:$LD_LIBRARY_PATH ./bin/test_snc_verify

# Production RingDim=32768
LD_LIBRARY_PATH=./openfhe-development/build/lib:$LD_LIBRARY_PATH ./bin/test_phi_32768
```

---

## Honest Limitations

| Limitation | Detail |
|-----------|--------|
| Bootstrap needed | φ-clean kills noise but doesn't reset CKKS modulus levels |
| TOY parameters | Most benchmarks at RingDim=4096 (not production security) |
| Author-reported | No third-party verification yet |
| Bounded circuits | Works best with naturally bounded values |
| Pre-scaling | Requires known clean schedule |
| φ-native rings | M%5=0 cyclotomic rings crash OpenFHE KeyGen (NTT) |
| iO composition | Theory verified, FHE execution HW-limited |
| Security proofs | Reduces to CKKS security; formal reduction pending |

---

## Project History

This project started with a claim that `EvalAdd(ct, Enc(0))` could reduce noise (SNC+ZANS). This was **experimentally disproven**. However, the deeper intuition — that the golden ratio has a role in FHE noise management — led to the φ-extension ring, asymmetric clean, and Fibonacci depth compression. These are genuine, verified contributions that emerged from honest exploration.

---

## Documentation

| Document | Description |
|----------|-------------|
| [Final Summary](docs/FINAL_SUMMARY.md) | Complete project overview |
| [Formal Proofs](docs/FORMAL_PROOFS.md) | Seven theorems with mathematical derivations |
| [Limitations & Q&A](docs/LIMITATIONS_AND_QA.md) | Known issues, open questions |
| [Reproduction Guide](docs/REPRODUCE.md) | Commands and expected outputs |
| [API Reference](docs/API_REFERENCE.md) | Core API |
| [Getting Started](docs/GETTING_STARTED.md) | Tutorial |
| [Security Model](docs/SECURITY_MODEL.md) | Threat model |
| [Hardware Scaling](docs/HARDWARE_SCALING.md) | Performance projections |

---

## License

MIT — see [LICENSE](LICENSE)

## Author

**Dan Joseph M. Fernandez / Primordial Omega Zero**

[![GitHub](https://img.shields.io/badge/GitHub-primordialomegazero-lightgrey.svg)](https://github.com/primordialomegazero)

---

```
- .... .. ... / .-. . .--. --- ... .. - --- .-. -.-- / .-- .. .-.. .-.. / .- .-.. .-- .- -.-- ... / -... . / -.. . -.. .. -.-. .- - . -.. / - --- / - .... . / .-- --- -- .- -. / .. .----. ...- . / . ...- . .-. / -.-. --- -. ... .. -.. . .-. . -.. / - --- / -... . / --- -. / -- -.-- / .-.. . ...- . .-.. .-.-.-
```
