# Comparison: This Prototype vs Established Schemes

**Version 2.0**

---

## Scope

This document compares the current prototype against established FHE and iO schemes. All measurements are from the test suite in this repository. These are small-scale results (N=1024, Q=2^29), not production benchmarks.

---

## FHE Comparison

### What This Prototype Implements

- RLWE encryption with golden ratio scaling (Q/φ instead of Q/2)
- 3-component ciphertext (c0, c1, c2) for exact multiplication
- Bootstrapping via decrypt-reencrypt
- Batch encryption (128 bits per ciphertext)

### What Established Schemes Have That This Lacks

| Feature | OpenFHE/TFHE | This Prototype |
|---------|--------------|----------------|
| NTT multiplication | Yes | No (naive O(N²)) |
| CRT batching | Yes | No |
| Key switching | Yes | No |
| Relinearization | Yes | No |
| Serialization | Yes | No |
| Formal security proofs | Yes | No |

### Small-Scale Measurements

| Metric | This Prototype | Notes |
|--------|---------------|-------|
| NAND (bootstrapped) | ~44 ops/sec | Naive multiplication |
| Batch encrypt | ~60K bits/sec | 128 bits/ciphertext |
| Bootstrap latency | ~4.2 ms | Simple decrypt-reencrypt |

These numbers are from `tests/test_full_benchmark.cpp`. They are included for reference; they do not represent production throughput.

---

## iO Comparison

### What This Prototype Implements

- Golden Orbit encoding: values on the unit circle (|value| = 1)
- Truth table obfuscation (2^n entries)
- Circuit obfuscation (NAND gates, O(n) size)
- No zero-test parameters

### What Established Schemes Have That This Lacks

| Feature | GGH13/CLT13 | This Prototype |
|---------|-------------|----------------|
| Formal indistinguishability proof | Attempted | No (empirical KS=0 only) |
| Matrix Branching Programs | Yes | No |
| Peer-reviewed security analysis | Yes | No |

### Why Previous iO Schemes Broke

GGH13, CLT13, and GGH15 all used zero-test parameters. Zeroizing attacks exploited these to recover secrets.

This prototype does not use zero-test parameters. All encoded values satisfy |value| = 1, so zero values do not occur.

This is a structural difference, not a claim of security.

---

## What the Tests Show

| Property | Result | Test File |
|----------|--------|-----------|
| FHE NAND correctness | Pass | `tests/test_fhe_fixed.cpp` |
| Bootstrapping depth | 20 levels | `tests/test_bootstrapping.cpp` |
| iO indistinguishability (KS) | 0 (100 pairs) | `tests/test_io_stress.cpp` |
| Circuit iO | 4-input XOR 16/16 | `tests/test_circuit_integrated_v2.cpp` |
| Attack resistance | 8 classes blocked | `tests/test_adversarial.cpp` |
| PRNG uniformity | Balance 0.0002 | `tests/test_golden_prng_inject.cpp` |

These tests pass at the current small scale. They do not validate security at production parameters.

---

## Honest Assessment

### What Is Demonstrated

- φ·ψ = -1 provides alternating signs in noise (observed in tests)
- Unit circle encoding eliminates zero values (by construction)
- Golden angle produces uniform distribution (tested)
- Small-scale correctness of NAND and bootstrapping

### What Is Not Demonstrated

- Security at Q=2^60+
- Performance with NTT/CRT
- Formal proofs
- Independent validation

### What This Is

A research prototype. It shows that the golden ratio approach is worth further study.

### What This Is Not

A production FHE library. Not a replacement for OpenFHE, TFHE, or SEAL. Not a formally verified scheme.

---

## Next Steps

For this approach to become a real FHE/iO scheme, the following are needed:

1. NTT polynomial multiplication
2. CRT batching
3. Key switching
4. Relinearization
5. Larger parameters (Q=2^60+)
6. Formal security analysis
7. Peer review

These are listed in ROADMAP.md.

---

## Contact

**Email:** devilswithin13@gmail.com

---

*φ · ψ = -1*
