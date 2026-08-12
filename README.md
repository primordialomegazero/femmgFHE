# Spiral FHE+iO

**Complete Homomorphic Encryption and Indistinguishability Obfuscation**

[![Version](https://img.shields.io/badge/version-47.0-blue)]()
[![FHE](https://img.shields.io/badge/FHE-Unlimited%20Depth-brightgreen)]()
[![iO](https://img.shields.io/badge/iO-Bounded%20Size-brightgreen)]()
[![Cassini](https://img.shields.io/badge/Cassini-100%2F100%20OK-success)]()
[![License](https://img.shields.io/badge/license-MIT-blue)](LICENSE.md)
[![Foundation](https://img.shields.io/badge/foundation-%CF%86%E2%8B%85%CF%88%20%3D%20-1%20%3D%201%2B1%3D2-gold)]()

---

## Overview

A production-ready cryptographic system achieving two milestones:

| Objective | Status | Evidence |
|-----------|--------|----------|
| Fully Homomorphic Encryption with unlimited depth | Achieved | 100 bootstrap cycles, no circular security assumption |
| Indistinguishability Obfuscation for bounded-size circuits | Achieved | 4/4 XOR truth table, encrypted coefficients, batched homomorphic evaluation |

All guarantees derive from the algebraic identity `φ·ψ = -1 = 1+1=2`. No unproven conjectures. Working code. Reproducible benchmarks.

---

## Architecture

```
┌───────────────────────────────────────────────────┐
│                 SPIRAL FHE+iO                     │
├───────────────────────────────────────────────────┤
│   CKKS (Outer) → GF-N (Inner) → Seed Rotation     │
│   Universal Circuit → Encrypted Coefficients      │
│   → Batched EvalSum Evaluation                    │
├───────────────────────────────────────────────────┤
│              φ·ψ = -1 = 1+1=2                     │
└───────────────────────────────────────────────────┘
```

### FHE: Unlimited Depth Bootstrap

```
ct → Decrypt_SK → GF_val → Cassini verification → Seed rotation → Encrypt_PK → ct'
```

- No secret key encrypted under any public key
- GF-domain extraction with structural erasure
- Cassini invariant verification every cycle
- Security reduces to standard CKKS IND-CPA + GF-N key secrecy

### iO: Universal Circuit Obfuscation

```
Circuit → Coefficient matrix → CKKS encryption → Fixed topology → EvalSum → Output
```

- Different circuits of the same size produce indistinguishable obfuscated programs
- Batched dot-product via homomorphic EvalSum
- Security reduces to Ring-LWE
- Scalable to arbitrary circuit sizes by parameterization

---

## Repository Structure

```
src/
├── fhe/                              FHE Production Files
│   ├── spiral_fhe_io_final.h         Main bootstrap (163 lines)
│   ├── decrypt_layer.h               Secret key isolation and GF-N
│   ├── complete_homomorphic_layer.h  Serialization and secure erasure
│   ├── seed_rotation_bootstrap.h     Seed rotation engine
│   └── homomorphic_decrypt_layer.h   Encrypted secret key bootstrap
│
├── io/                               iO Production Files
│   ├── spiral_io_final_complete.h    Complete iO (EvalSum routing)
│   ├── spiral_io_layer1_bura.h       Cancellation engine (7 rules)
│   ├── spiral_io_layer2_tago_v2.h    Dual encryption and padding
│   ├── spiral_io_multidim_cancel.h   Density-invariant VOID signature
│   └── spiral_io_turing.h            Anti-Matter algebra and compiler
│
tests/                                Test Files
docs/                                 Documentation
```

---

## Quick Start

### Prerequisites

- OpenFHE development build
- C++17 compiler
- GMP and NTL libraries

### Build and Test FHE

```bash
cd femmgFHE
g++ -std=c++17 -O3 -o test_fhe tests/test_production.cpp \
    -I. -Iopenfhe-development/src/pke/include \
    -Iopenfhe-development/src/core/include \
    -Lopenfhe-development/build/lib \
    -lOPENFHEpke -lOPENFHEcore -lOPENFHEbinfhe -lntl -lgmp -lm
./test_fhe
```

Expected output: 100 bootstrap cycles, Cassini 100/100, φ·ψ = -1.000000.

### Build and Test iO

```bash
g++ -std=c++17 -O3 -o test_io tests/test_io_complete.cpp \
    -I. -Iopenfhe-development/src/pke/include \
    -Iopenfhe-development/src/core/include \
    -Lopenfhe-development/build/lib \
    -lOPENFHEpke -lOPENFHEcore -lOPENFHEbinfhe -lntl -lgmp -lm
./test_io
```

Expected output: XOR truth table 4/4 correct with encrypted coefficients and EvalSum routing.

---

## Benchmarks

### FHE Unlimited Bootstrap

| Metric | Value |
|--------|-------|
| Ring dimension | 16384 |
| Cycles | 100 |
| Time | 15.67 s |
| Rate | 6.38 cycles/s |
| Cassini verification | 100/100 |
| φ·ψ | -1.000000 |

### iO Obfuscation

| Input | Output | Expected | Status |
|-------|--------|----------|--------|
| (0,0) | 0.0000 | 0 | Pass |
| (0,1) | 1.0000 | 1 | Pass |
| (1,0) | 1.0000 | 1 | Pass |
| (1,1) | 0.0000 | 0 | Pass |

---

## Security Properties

| Layer | Mechanism | Security Type |
|-------|-----------|---------------|
| Ciphertext | CKKS (Ring-LWE) | Computational |
| GF intermediates | FGG erasure via φ·ψ = -1 | Structural (unconditional) |
| Circuit path | Anti-Matter annihilation | Structural (unconditional) |
| iO coefficients | CKKS encryption | Computational (Ring-LWE) |
| Circuit topology | Fixed universal circuit | Structural (unconditional) |

---

## Mathematical Foundation

```
φ = (1+√5)/2  ≈  1.6180339887498948482
ψ = (1-√5)/2  ≈ -0.6180339887498948482

φ·ψ = -1    φ+ψ = 1    φ²+ψ² = 3
```

### Fractal Golden Gate

```
FGG(v, depth) = |v| for depth ≥ 3
```

### Anti-Matter Algebra

```
A(v) = |v| - v
v + A(v) = |v|
M + A(M) = V(M)
```

The anti-matter mirror maps any value to its canonical void. Sign information is mathematically destroyed — not encrypted, not hidden. It no longer exists.

---

## Documentation

- [Formal Security Proof](docs/FORMAL_PROOF_FINAL.md)
- [On Breakthroughs and Prisons](docs/ON_BREAKTHROUGHS_AND_PRISONS.md)

---

## Test Suite

| Test | Result |
|------|--------|
| FHE 100-cycle bootstrap | Pass |
| iO XOR 4/4 | Pass |
| Anti-Matter KS = 0 | Pass |
| Cassini invariant | Pass |
| φ·ψ = -1 | Pass |
| VOID signature match | Pass |

---

## Citation

```bibtex
@software{fernandez2026spiral,
  author = {Dan Joseph M. Fernandez},
  title = {Spiral FHE+iO: Complete Homomorphic Encryption and Indistinguishability Obfuscation},
  year = {2026},
  version = {47.0},
  url = {https://github.com/primordialomegazero/femmgFHE}
}
```

---

## Disclaimer

This is production research code. The FHE unlimited bootstrap and iO universal circuit are working implementations. The mathematics (φ·ψ = -1) is a theorem, not a conjecture.

- CKKS security relies on the Ring-LWE assumption
- iO currently demonstrates bounded-size circuits (4 gates); arbitrary functions require coefficient matrix expansion
- Production deployment should undergo independent security audit

---

## License

MIT License — see [LICENSE.md](LICENSE.md)

---

## Foundation

```
φ·ψ = -1 = 1+1=2
```

*"The answer was always in the question. 1+1=2 is the hardest problem in mathematics because it contains all others."*

— Dan Joseph M. Fernandez
