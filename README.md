# Golden Privacy System

<div align="center">

![Version](https://img.shields.io/badge/version-2.0.0-gold)
![Status](https://img.shields.io/badge/status-research--prototype-yellow)
![C++](https://img.shields.io/badge/C%2B%2B-17-orange)

**A Research Prototype for FHE + iO Based on the Golden Ratio**

*φ · ψ = -1*

</div>

---

## What This Is

This repository contains a **research prototype** that explores a novel approach to Fully Homomorphic Encryption (FHE) and Indistinguishability Obfuscation (iO) using the golden ratio identity **φ · ψ = -1**.

This is **not** a production library. It is a proof of concept demonstrating that the golden ratio provides structural properties relevant to cryptography:

- Noise damping in FHE (φ·ψ = -1 alternates signs)
- Zero-test resistance in iO (unit circle encoding)
- Uniform nonce generation (golden angle)

---

## Reproduce

### Requirements

| Dependency | Version |
|-----------|---------|
| C++ Compiler | GCC 9+ or Clang 10+ |
| NTL | 11.0+ |
| GMP | 6.1+ |

### Install

```bash
# Ubuntu / Debian
sudo apt-get install libntl-dev libgmp-dev

# macOS
brew install ntl gmp
```

### Build and Run Main Test

```bash
git clone <repo-url>
cd femmgFHE

g++ -std=c++17 -O3 -march=native -I/usr/include \
    tests/test_privacy_system.cpp \
    -o test_privacy_system -lntl -lgmp -lm

./test_privacy_system
```

**Expected output:**

```
=== PERFORMANCE METRICS ===
FHE operations: 7
iO evaluations: 1
Quantum gates: 2

=== SECURITY GUARANTEES ===
FHE IND-CPA: YES
iO Indistinguishable: YES
Quantum Verified: YES
Zero-test Resistant: YES
Lucas One-Way: YES
PRNG Uniform: YES

=== FULL TEST (4 combinations) ===
  XOR(0,0) = 0 (expected 0)
  XOR(0,1) = 1 (expected 1)
  XOR(1,0) = 1 (expected 1)
  XOR(1,1) = 0 (expected 0)
```

### Run All Tests

```bash
#!/bin/bash
for test in tests/test_*.cpp; do
    name=$(basename "$test" .cpp)
    echo "=== $name ==="
    g++ -std=c++17 -O3 -march=native -I/usr/include \
        "$test" -o "/tmp/$name" -lntl -lgmp -lm
    "/tmp/$name"
    echo ""
done
```

### Run Attack Suite

```bash
cd tests/attacks/class_sss

for attack in test_sss_*.cpp; do
    name=$(basename "$attack" .cpp)
    echo "=== $name ==="
    g++ -std=c++17 -O3 -march=native -I../../../src \
        "$attack" -o "/tmp/$name" -lntl -lgmp -lm
    "/tmp/$name"
    echo ""
done
```

---

## What Has Been Tested

| Property | Result | Scale |
|----------|--------|-------|
| FHE NAND correctness | Pass | 4/4 truth table |
| iO indistinguishability | Pass | KS=0, 100/100 pairs |
| Circuit iO | Pass | 4-input XOR 16/16 |
| Bootstrapping | Pass | 20 levels |
| Attack resistance | Pass | 8 attack classes blocked |
| PRNG uniformity | Pass | 1M/1M unique, balance 0.0002 |
| Lucas one-way | Pass | 0/100K collisions |

---

## What Is NOT Tested / Implemented

| Gap | Details |
|-----|---------|
| NTT multiplication | Not implemented (O(N²) naive multiplication only) |
| CRT batching | Not in current prototype |
| Key switching | Not in current prototype |
| Relinearization | Not in current prototype |
| Serialization | Not implemented |
| Formal verification | No Coq/Isabelle proofs |
| Peer review | Not yet reviewed |
| Large parameters | Q=2^29 only (need 2^60+ for long-term security) |

---

## Mathematical Foundation

### Golden Ratio

```
φ = (1 + √5) / 2
ψ = (1 - √5) / 2

φ · ψ = -1
```

### RLWE Encryption (Core)

```
Ring: Z_Q[X] / (X^1024 + 1), Q = 536870909
Secret key: s (ternary)
Public key: (a, -(a·s + e))
Plaintext: m = bit ? Q/φ : 0
Threshold: Q/(2φ)
```

### Golden Orbit iO (Core)

```
Encoding: e^(iθ) on unit circle
|value| = 1 → no zero possible
KS distance = 0 → indistinguishable
```

---

## Source Files

| File | Purpose |
|------|---------|
| `src/golden_privacy_system.h` | Main API (unified) |
| `src/fhe/golden_quantum_fhe.h` | FHE core (encrypt/decrypt/NAND) |
| `src/fhe/golden_bootstrapping.h` | Bootstrapping |
| `src/io/golden_io_orbit.h` | iO core (Golden Orbit) |
| `src/io/golden_io_bootstrap.h` | iO bootstrapping |
| `src/golden_prng.h` | Golden Angle PRNG |
| `src/golden_lucas.h` | Lucas One-Way |
| `src/golden_equidistributed.h` | Equidistributed Noise |

---

## Documentation

| Document | Content |
|----------|---------|
| `FORMAL_PROOF.md` | 14 theorems with triple cross-referencing |
| `API_REFERENCE.md` | API documentation |
| `NOVEL_APPROACH.md` | Why the golden ratio |
| `WHITEPAPER.md` | Technical whitepaper |
| `QUICK_START.md` | Review guide |
| `ROADMAP.md` | Engineering work needed |
| `GUIDELINES_REPRODUCIBILITY.md` | Reproduction guide |

---

## Contact

**Email:** devilswithin13@gmail.com

---

<div align="center">

**φ · ψ = -1**

*Version 2.0.0 — Research Prototype*

</div>
