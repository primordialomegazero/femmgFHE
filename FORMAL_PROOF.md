# Formal Claims and Test Evidence

**Version 3.0**

---

## How to Read This Document

Each claim below includes:

- **Source code reference** — where the implementation is
- **Test reference** — where the behavior is tested
- **Status** — whether the claim is proven, tested, or assumed

This document does not claim formal security proofs. It documents what has been tested at small scale and what remains as assumptions.

---

## Claim Index

| # | Claim | Status | Source | Test |
|---|-------|--------|--------|------|
| 1 | IND-CPA under RLWE | Assumed | `src/fhe/golden_quantum_fhe.h` | `tests/test_adversarial.cpp` |
| 2 | NAND correctness | Tested | `src/fhe/golden_quantum_fhe.h` | `tests/test_fhe_fixed.cpp` |
| 3 | Bootstrapping correctness | Tested | `src/fhe/golden_bootstrapping.h` | `tests/test_bootstrapping.cpp` |
| 4 | iO indistinguishability | Tested (KS=0) | `src/io/golden_io_orbit.h` | `tests/test_io_stress.cpp` |
| 5 | Zero-test resistance | By construction | `src/io/golden_io_orbit.h` | `tests/test_adversarial.cpp` |
| 6 | Timing resistance | Tested | `src/golden_privacy_system.h` | `tests/test_timing_final.cpp` |
| 7 | Batch encryption | Tested | `src/golden_privacy_system.h` | `tests/test_full_benchmark.cpp` |
| 8 | φ·ψ = -1 | Algebraic identity | `src/fhe/golden_quantum_fhe.h` | N/A |
| 9 | Noise damping | Observed | `src/fhe/golden_quantum_fhe.h` | Archive tests |
| 10 | Full pipeline | Tested | `src/golden_privacy_system.h` | `tests/test_privacy_system.cpp` |
| 11 | Circuit obfuscation | Tested | `src/io/golden_io_orbit.h` | `tests/test_circuit_integrated_v2.cpp` |
| 12 | PRNG uniformity | Tested | `src/golden_prng.h` | `tests/test_golden_prng_inject.cpp` |
| 13 | Lucas commitment | Tested | `src/golden_lucas.h` | `tests/test_lucas_inject.cpp` |
| 14 | Noise equidistribution | Tested | `src/golden_equidistributed.h` | `tests/test_equidistributed_inject.cpp` |

---

## Status Definitions

| Status | Meaning |
|--------|---------|
| **Assumed** | Relies on a standard hardness assumption (e.g., RLWE). No independent proof provided. |
| **Tested** | Behavior verified by test suite at small scale (N=1024, Q=2^29). |
| **By construction** | Property follows directly from the mathematical structure. |
| **Algebraic identity** | Standard mathematical fact, not a security claim. |
| **Observed** | Behavior seen in testing; no formal proof. |

---

## Claim 1: IND-CPA under RLWE

**Status:** Assumed

**Basis:** The public key is an RLWE sample `(a, -(a·s + e))`. Under the RLWE assumption, distinguishing this from uniform is hard.

**Not provided:** A formal reduction proof. This prototype relies on the standard RLWE assumption without independent verification.

---

## Claim 2: NAND Correctness

**Status:** Tested

**Test:** `tests/test_fhe_fixed.cpp` — 4/4 truth table cases pass.

| a | b | Result | Expected |
|---|---|--------|----------|
| 0 | 0 | 1 | 1 |
| 0 | 1 | 1 | 1 |
| 1 | 0 | 1 | 1 |
| 1 | 1 | 0 | 0 |

---

## Claim 3: Bootstrapping

**Status:** Tested

**Test:** `tests/test_bootstrapping.cpp` — 20-level NOT chain, all correct.

**Not provided:** Formal proof that decrypt-reencrypt bootstrapping preserves IND-CPA security. This is a known gap.

---

## Claim 4: iO Indistinguishability

**Status:** Tested (empirical)

**Test:** `tests/test_io_stress.cpp` — 100 pairs of different functions, KS distance = 0.

**Not provided:** A formal indistinguishability proof. The KS=0 result is empirical, not a security proof.

---

## Claim 5: Zero-test Resistance

**Status:** By construction

**Basis:** All encoded values satisfy |value| = 1. Zero values cannot occur.

**Not provided:** A formal proof that absence of zero values implies security against all zeroizing variants.

---

## Claim 6: Timing Resistance

**Status:** Tested

**Test:** `tests/test_timing_final.cpp` — no data-dependent timing correlation found.

**Not provided:** A formal constant-time analysis.

---

## Claim 7: Batch Encryption

**Status:** Tested

**Test:** `tests/test_full_benchmark.cpp` — batch encrypt/decrypt works for 128 bits per ciphertext.

---

## Claim 8: φ·ψ = -1

**Status:** Algebraic identity

**Proof:**
```
φ = (1 + √5) / 2
ψ = (1 - √5) / 2
φ·ψ = ((1+√5)/2)((1-√5)/2) = (1-5)/4 = -1
```

This is standard mathematics, not a security claim.

---

## Claim 9: Noise Damping

**Status:** Observed

**Basis:** In tests, noise values remain at golden_plain or 0 after 1000 operations.

**Not provided:** A formal proof that the alternation from φ·ψ = -1 bounds noise in all cases.

---

## Claim 10: Full Pipeline

**Status:** Tested

**Test:** `tests/test_privacy_system.cpp` — 4/4 XOR cases pass through FHE → iO → Quantum → FHE.

---

## Claim 11: Circuit Obfuscation

**Status:** Tested

**Test:** `tests/test_circuit_integrated_v2.cpp`

| Circuit | Gates | Truth Table |
|---------|-------|-------------|
| 4-input XOR | 12 | 16 |
| 8-input AND | 14 | 256 |

---

## Claim 12: PRNG Uniformity

**Status:** Tested

**Test:** `tests/test_golden_prng_inject.cpp`

| Metric | Result |
|--------|--------|
| Uniqueness | 1M/1M |
| Balance | 0.0002 |

**Not claimed:** Cryptographic security of the PRNG. This is a deterministic sequence with good distribution, not a CSPRNG.

---

## Claim 13: Lucas Commitment

**Status:** Tested

**Test:** `tests/test_lucas_inject.cpp`

| Metric | Result |
|--------|--------|
| Collisions | 0/100K |
| Tamper detection | Works |

**Not claimed:** Formal binding/hiding proofs. No reduction to a hard problem is provided.

---

## Claim 14: Noise Equidistribution

**Status:** Tested

**Test:** `tests/test_equidistributed_inject.cpp`

| Metric | Result |
|--------|--------|
| Balance | 0.0002 |

---

## What Is NOT Claimed

- Formal security proofs (Coq/Isabelle)
- Security at production parameters (Q=2^60+)
- CSPRNG status for the PRNG
- Formal iO indistinguishability
- Formal commitment scheme security
- Resistance to attacks not yet considered

---

## Contact

**Email:** devilswithin13@gmail.com

---

*This document is a factual record of what has been tested. It is not a substitute for peer-reviewed security analysis.*
