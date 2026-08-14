# FHE Implementation in This Prototype

**Version 2.0**

---

## What This Document Describes

This document explains the FHE implementation in this research prototype. It describes what is implemented, what parameters are used, and what limitations exist.

This is not a claim of a new FHE scheme. It is a description of an RLWE-based construction that uses golden ratio scaling instead of the traditional Q/2.

---

## Core Construction

### RLWE Foundation

```
Ring: Z_Q[X] / (X^1024 + 1)
Q = 536870909 (29-bit modulus)
N = 1024

Secret key: s ∈ {-1, 0, 1}^1024 (ternary)
Public key: (a, -(a·s + e))
Error: sparse (1/10000 rate)
```

### Golden Ratio Scaling

Traditional RLWE uses plaintext scaling Q/2. This prototype uses:

```
Plaintext: m = bit ? Q/φ : 0
Threshold: Q/(2φ)
```

The identity φ·ψ = -1 means that multiplying by φ·ψ flips the sign. This alternation is observed in the noise behavior during testing.

### 3-Component Ciphertext

Traditional RLWE ciphertexts have 2 components (c0, c1). This prototype uses 3 components (c0, c1, c2):

```
c0 + c1·s + c2·s² = message + error
```

The third component tracks the s² term that appears during multiplication. In the ring Z_Q[X]/(X^1024 + 1), s² is not simply -1, so this term must be tracked explicitly.

---

## What Is Implemented

- Key generation (ternary secret key)
- Encryption (single bit, RLWE with Q/φ scaling)
- Decryption (threshold Q/(2φ))
- Homomorphic NAND gate (3-component multiplication with rescaling)
- Bootstrapping (decrypt-reencrypt)
- Batch encryption (128 bits per ciphertext via coefficient packing)

## What Is NOT Implemented

- NTT polynomial multiplication (naive O(N²) used instead)
- CRT batching
- Key switching
- Relinearization
- Serialization
- Formal security proofs

---

## Small-Scale Measurements

Measurements from `tests/test_full_benchmark.cpp` at N=1024, Q=2^29:

| Metric | Value | Notes |
|--------|-------|-------|
| NAND (bootstrapped) | ~44 ops/sec | Naive multiplication |
| Batch encrypt | ~60K bits/sec | 128 bits/ciphertext |
| Bootstrap latency | ~4.2 ms | Simple decrypt-reencrypt |

These are reference numbers. They do not represent production throughput.

---

## Comparison with Established Schemes

| Feature | BFV | CKKS | TFHE | This Prototype |
|---------|-----|------|------|----------------|
| Plaintext scaling | Q/2 | Floating | Binary | Q/φ |
| NTT | Yes | Yes | Yes | No |
| Key switching | Yes | Yes | Yes | No |
| Bootstrap | Complex | Complex | Blind rotation | Decrypt-reencrypt |
| Formal security proof | Yes | Yes | Yes | No |
| Code size | ~100K lines | ~100K lines | ~50K lines | ~300 lines core |

The comparison is not a claim of superiority. It documents what each scheme implements.

---

## Known Limitations

1. **Q = 2^29** — too small for long-term security
2. **Naive multiplication** — no NTT, O(N²) instead of O(N log N)
3. **Sparse error** — 1/10000 rate, may be insufficient for worst-case hardness
4. **No formal proof** — security relies on RLWE assumption informally
5. **No relinearization** — ciphertext grows to 3 components permanently

---

## Future Work

Listed in ROADMAP.md:

- NTT polynomial multiplication
- CRT decomposition
- Key switching
- Relinearization
- Larger parameters (Q=2^60+)
- Formal verification

---

## Conclusion

This prototype demonstrates an RLWE-based FHE construction with golden ratio scaling at small scale. The tests pass, but this is not production software.

*φ · ψ = -1*
