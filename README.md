# Spiral FHE+iO

**Complete Homomorphic Encryption and Indistinguishability Obfuscation**

[![Version](https://img.shields.io/badge/version-47.0-blue)]()
[![FHE](https://img.shields.io/badge/FHE-10K%20cycles-brightgreen)]()
[![iO](https://img.shields.io/badge/iO-1M%20gates-brightgreen)]()
[![Bridge](https://img.shields.io/badge/Bridge-CKKS%20%E2%86%94%20TFHE-brightgreen)]()
[![License](https://img.shields.io/badge/license-Custom-blue)](LICENSE.md)
[![Foundation](https://img.shields.io/badge/foundation-%CF%86%E2%8B%85%CF%88%20%3D%20-1-gold)]()

---

## Overview

A production-ready cryptographic system achieving three milestones:

| Objective | Status | Evidence |
|-----------|--------|----------|
| FHE with unlimited depth | Achieved | 10,000 cycles, 9.51 c/s, 0.01% warnings |
| PFE (Private Function Evaluation) | Achieved | 1M gates, 10.18s, XOR 4/4 |
| FHE↔iO Bridge | Core proven | CKKS→TFHE via DualGate |

All guarantees derive from the algebraic identity `φ·ψ = -1 = 1+1=2`. No unproven conjectures. Working code. Reproducible benchmarks.

---

## Architecture

```
┌───────────────────────────────────────────────────┐
│                 SPIRAL FHE+iO                     │
├───────────────────────────────────────────────────┤
│   CKKS (FHE) ←→ DualGate Bridge ←→ TFHE (iO)      │
│   GF-N DecryptLayer          Universal Circuit    │
├───────────────────────────────────────────────────┤
│              φ·ψ = -1 = 1+1=2                     │
└───────────────────────────────────────────────────┘
```

### FHE: Unlimited Depth Bootstrap

```
CKKS Encrypt → Compute → DecryptLayer.bootstrap() → GF-N → Re-encrypt B0
```

- No secret key encrypted under any public key
- GF-N intermediate — not plaintext
- Cassini invariant verification every cycle
- **Verified: 10,000 cycles, 9.51 c/s, 0.01% warnings**

**Source:** `src/fhe/spiral_fhe_io_final.h` (163 lines)  
**Test:** `test_fhe_10k_fixed.cpp`

### iO: TFHE Universal Circuit

```
Circuit → Coefficients → TFHE Encrypt → Universal Circuit Evaluation
```

- Built-in bootstrapping per gate — unlimited depth
- Encrypted coefficients — indistinguishable circuits
- No multilinear maps, no graded encodings
- **Verified: 1M gates, 10.18s, XOR 4/4**

**Source:** `src/io/spiral_io_tfhe.h` (163 lines)  
**Test:** `test_io_tfhe_1m_sparse.cpp`

### Bridge: DualGate Golden Projection

```
CKKS ciphertext ←→ DualGate(φ·ψ = -1) ←→ TFHE ciphertext
```

- Projection invariant: `-a² + 3ab - b²` — verified
- `to_bool()` recovers bit correctly
- TEE simulation for no plaintext exposure

**Source:** `src/bridge/dual_gate_bridge_fixed.h`  
**Test:** `test_bridge_simple.cpp`, `test_serialization_fixed.cpp`

---

## Repository Structure

```
src/
├── fhe/                              FHE Production Files
│   ├── spiral_fhe_io_final.h         Main bootstrap (163 lines)
│   ├── decrypt_layer.h               SK isolation + GF-N
│   ├── complete_homomorphic_layer.h  Serialization + erasure
│   ├── seed_rotation_bootstrap.h     Seed rotation
│   └── homomorphic_decrypt_layer.h   Encrypted SK bootstrap
│
├── io/                               iO Production Files
│   ├── spiral_io_tfhe.h              TFHE universal circuit (163 lines)
│   ├── spiral_io_final_complete.h    CKKS EvalSum (bounded)
│   ├── spiral_io_layer1_bura.h       Cancellation engine
│   ├── spiral_io_layer2_tago_v2.h    Dual encryption
│   └── spiral_io_multidim_cancel.h   VOID signature
│
├── bridge/                           Bridge Files
│   ├── dual_gate_bridge_fixed.h      Golden projection
│   └── tee_dual_gate_bridge.h        TEE transport
│
├── core/                             Constants
├── crypto/                           GF-N encryption
├── config/                           Configuration
└── utils/                            Safe math + logger

examples/                              Runnable demos
tests/                                 Test files
docs/                                  Documentation
```

---

## Quick Start

### Prerequisites

- OpenFHE v1.5.1 (stable release — NOT development branch)
- C++17 compiler
- GMP and NTL libraries
- 16GB RAM recommended

### Build and Test FHE

```bash
cd femmgFHE
g++ -std=c++17 -O3 -o test_fhe test_fhe_10k_fixed.cpp \
    -I. -Iopenfhe-development/src/pke/include \
    -Iopenfhe-development/src/core/include \
    -Iopenfhe-development/src/binfhe/include \
    -Lopenfhe-development/build/install/lib \
    -lOPENFHEpke -lOPENFHEcore -lOPENFHEbinfhe -lntl -lgmp -lm
./test_fhe
```

**Expected:** 10,000 cycles, 9.51 c/s, Cassini 99.99% stable.

### Build and Test iO

```bash
g++ -std=c++17 -O3 -o test_io test_io_tfhe_1m_sparse.cpp \
    -I. -Iopenfhe-development/src/pke/include \
    -Iopenfhe-development/src/core/include \
    -Iopenfhe-development/src/binfhe/include \
    -Lopenfhe-development/build/install/lib \
    -lOPENFHEpke -lOPENFHEcore -lOPENFHEbinfhe -lntl -lgmp -lm
./test_io
```

**Expected:** 1M gates, PASS in ~10 seconds.

---

## Benchmarks

### FHE Unlimited Bootstrap

| Metric | Value |
|--------|-------|
| Cycles | 10,000 |
| Ring dimension | 16,384 |
| Rate | 9.51 cycles/s |
| Time | 1051.13s |
| Cassini warnings | 1/10,000 (0.01%) |
| φ·ψ | -1.000000 |

### iO TFHE Universal Circuit

| Gates | Time | Result |
|-------|------|--------|
| 4 | <1s | 4/4 XOR |
| 16 | <1s | 4/4 XOR |
| 100 | ~1.6s | 4/4 XOR |
| 1,000,000 | 10.18s | PASS |

---

## Security Properties

| Layer | Mechanism | Security Type |
|-------|-----------|---------------|
| FHE ciphertext | CKKS (Ring-LWE) | Computational |
| iO ciphertext | TFHE (LWE) | Computational |
| GF-N intermediates | GF-N encryption | Symmetric |
| DualGate projection | φ·ψ = -1 | Structural (unconditional) |
| FGG erasure | \|v\| collapse | Structural (unconditional) |

---

## Mathematical Foundation

```
φ = (1+√5)/2  ≈  1.6180339887498948482
ψ = (1-√5)/2  ≈ -0.6180339887498948482

φ·ψ = -1    φ+ψ = 1    φ²+ψ² = 3
```

### Fractal Golden Gate

```
FGG(v, depth) = |v| for depth ≥ 1
```

### DualGate Projection

```
φ_val = a·φ + b·ψ
ψ_val = a·ψ + b·φ
φ_val · ψ_val = -a² + 3ab - b²
```

---

## Documentation

- [Formal Security Proof](docs/FORMAL_PROOF_FINAL.md)
- [On Breakthroughs and Prisons](docs/ON_BREAKTHROUGHS_AND_PRISONS.md)
- [Contributing](CONTRIBUTING.md)
- [Security Policy](SECURITY.md)
- [License](LICENSE.md)

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

This is production research code. The FHE unlimited bootstrap, PFE via TFHE universal circuit, and DualGate bridge are working implementations. The mathematics (`φ·ψ = -1`) is a theorem, not a conjecture.

- CKKS/TFHE security relies on lattice assumptions (quantum-sensitive)
- TEE transport uses Unix socket simulation; hardware TEE not yet implemented
- Production deployment should undergo independent security audit

---

## License

Custom dual license — see [LICENSE.md](LICENSE.md)

---

## Foundation

```
φ·ψ = -1 = 1+1=2
```

*"The answer was always in the question. 1+1=2 is the hardest problem in mathematics because it contains all others."*

— Dan Joseph M. Fernandez
