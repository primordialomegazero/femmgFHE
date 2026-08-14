# Technical Documentation

**Version 2.0**

---

## 1. Overview

This repository contains a research prototype for FHE and iO using the golden ratio identity φ·ψ = -1.

Scope: small-scale implementation (N=1024, Q=2^29). Not production software.

---

## 2. Implemented Components

| Component | Description | Status |
|-----------|-------------|--------|
| FHE core | RLWE encrypt/decrypt, NAND | Tested |
| Bootstrapping | Decrypt-reencrypt refresh | Tested |
| iO Golden Orbit | Unit circle encoding | Tested |
| Circuit iO | NAND-based circuits | Tested |
| Golden Angle PRNG | Deterministic uniform sequence | Tested |
| Lucas commitment | φ^n + ψ^n rounding | Tested |
| Equidistributed noise | Golden angle addition | Tested |

---

## 3. NOT Implemented

| Feature | Notes |
|---------|-------|
| NTT multiplication | Naive O(N²) used instead |
| CRT batching | Not in current prototype |
| Key switching | Not in current prototype |
| Relinearization | Not in current prototype |
| Serialization | Not implemented |
| Formal proofs | Informal only |

---

## 4. Mathematical Details

### Golden Ratio

```
φ = (1 + √5) / 2
ψ = (1 - √5) / 2
φ · ψ = -1
```

### RLWE Parameters

```
Ring: Z_Q[X] / (X^1024 + 1)
Q = 536870909 (29-bit)

Secret key: ternary {-1, 0, 1}
Public key: (a, -(a·s + e))
Error: sparse (1/10000)

Plaintext: m = bit ? Q/φ : 0
Threshold: Q/(2φ)
```

### Golden Orbit Encoding

```
value = e^(iθ)
TRUE  → θ ∈ (0, π)
FALSE → θ ∈ (π, 2π)

|value| = 1 for all θ
```

---

## 5. Complexity

| Operation | Time | Space |
|-----------|------|-------|
| Encrypt | O(N) | O(N) |
| Decrypt | O(N²) | O(N) |
| NAND | O(N²) | O(N) |
| Bootstrap | O(N²) | O(N) |
| iO truth table obfuscate | O(2^n) | O(2^n) |
| iO circuit obfuscate | O(n) | O(n) |
| iO evaluate | O(n) | O(1) |

Note: O(N²) is due to naive polynomial multiplication. NTT would reduce this to O(N log N).

---

## 6. Dependencies

| Library | Version |
|---------|---------|
| NTL | 11.0+ |
| GMP | 6.1+ |
| C++ Standard | C++17 |

---

## 7. System Requirements

| Component | Minimum |
|-----------|---------|
| CPU | 64-bit, 2 cores |
| RAM | 4 GB |
| OS | Linux (Ubuntu 20.04+) or macOS 12+ |

---

## 8. Limitations

- Q = 2^29 is small; need 2^60+ for long-term security
- Naive multiplication is slow
- No formal security proofs
- Not peer-reviewed
- Small scale only (2-4 input functions tested)

---

## 9. Build

```bash
g++ -std=c++17 -O3 -march=native -I/usr/include \
    tests/test_privacy_system.cpp \
    -o test_privacy_system -lntl -lgmp -lm
```

---

*This documentation is accurate as of Version 2.0. It reflects the current small-scale prototype, not a production system.*
