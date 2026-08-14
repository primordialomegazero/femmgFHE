# System Overview

**Version 2.0**

---

## What This Is

This repository contains a research prototype for FHE and iO based on the golden ratio identity φ·ψ = -1.

The prototype has multiple components, each tested at small scale (N=1024, Q=2^29).

---

## Components

| Component | File | Status |
|-----------|------|--------|
| FHE core | `src/fhe/golden_quantum_fhe.h` | Tested |
| Bootstrapping | `src/fhe/golden_bootstrapping.h` | Tested |
| iO (Golden Orbit) | `src/io/golden_io_orbit.h` | Tested |
| iO bootstrapping | `src/io/golden_io_bootstrap.h` | Tested |
| Golden Angle PRNG | `src/golden_prng.h` | Tested |
| Lucas commitment | `src/golden_lucas.h` | Tested |
| Equidistributed noise | `src/golden_equidistributed.h` | Tested |
| Main API | `src/golden_privacy_system.h` | Tested |

---

## What Is Implemented

### FHE

- RLWE encryption with Q/φ plaintext scaling
- 3-component ciphertext (c0, c1, c2)
- Homomorphic NAND
- Bootstrapping via decrypt-reencrypt
- Batch encryption (128 bits per ciphertext)

### iO

- Golden Orbit encoding (unit circle)
- Truth table mode (2^n entries)
- Circuit mode (O(n) NAND gates)
- No zero-test parameters

### Other Components

- Golden Angle PRNG (deterministic uniform sequence)
- Lucas commitment (φ^n + ψ^n rounding)
- Equidistributed noise (golden angle addition)

---

## What Is NOT Implemented

- NTT polynomial multiplication
- CRT batching
- Key switching
- Relinearization
- Serialization
- Formal security proofs

These are listed in ROADMAP.md.

---

## Mathematical Foundation

```
φ = (1 + √5) / 2
ψ = (1 - √5) / 2
φ · ψ = -1
```

Ring: Z_Q[X] / (X^1024 + 1), Q = 536870909.

---

## Test Results

| Component | Test | Result |
|-----------|------|--------|
| FHE NAND | `tests/test_fhe_fixed.cpp` | Pass |
| Bootstrapping | `tests/test_bootstrapping.cpp` | Pass (20 levels) |
| iO indistinguishability | `tests/test_io_stress.cpp` | KS=0 (100 pairs) |
| Circuit iO | `tests/test_circuit_integrated_v2.cpp` | Pass (16/16) |
| Attack resistance | `tests/test_adversarial.cpp` | 8/8 blocked |
| PRNG uniformity | `tests/test_golden_prng_inject.cpp` | Balance 0.0002 |
| Lucas commitment | `tests/test_lucas_inject.cpp` | 0/100K collisions |

---

## Honest Summary

This is a small-scale prototype. The tests pass. The approach uses φ·ψ = -1 as a structural foundation. Whether this provides security beyond standard assumptions is an open question.

This is not production software. See README.md for full context.

---

*φ · ψ = -1*
