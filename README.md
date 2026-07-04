# FEmmg-FHE — Empirical FHE Noise Analysis

**License:** MIT | **C++17** | **SEAL 4.3.3** | **BFV Scheme**

## What This Is

A set of empirical observations on FHE noise behavior, measured on real Microsoft SEAL BFV ciphertexts. No new cryptography. No security claims beyond standard SEAL.

---

## Observations

All measurements on real SEAL BFV ciphertexts (poly_modulus_degree=16384, unless noted).

### 1. Addition Noise Drift (ZANS)

Repeated `ct + Enc(0)` operations show noise drift significantly lower than expected.

| Metric | Value |
|--------|-------|
| Operations tested | 10,000 |
| Total noise drift | 13 bits |
| Drift per operation | 0.0013 bits/op |
| Expected drift (theory) | ~1 bit/op |
| Value preservation | 100% |

**Open question:** Theory predicts ~1 bit/op noise growth for BFV addition. We observe 0.0013 bits/op. The mechanism is unclear.

### 2. UK×UK Chain Length

UK×UK multiplication chain length with ZANS stabilization.

| Metric | Value |
|--------|-------|
| Chain length (no bootstrap) | 11 ops |
| Standard SEAL BFV | 1-2 ops |
| Noise per UK×UK | 32.6 bits |

### 3. UK×PT Chain Length

Known-multiplier chain length.

| Metric | Value |
|--------|-------|
| Chain length (20-bit plain) | 14-29 ops |

### 4. Fibonacci Decomposition

Zeckendorf decomposition verified correct for multipliers up to 731 trillion.

```
a × b = Σ (a × F_i)    where b = Σ F_i
Complexity: O(log_φ n)
```

Does not improve noise for known multiplies vs direct multiply_plain.

### 5. Fibonacci Modulus Chain (Standalone)

Mathematical concept only — numbers do not satisfy SEAL modulus requirements (not all prime, not all ≡1 mod 2N).

```
Chain: 65537 → 46368 → 28657 → 17711 → 10946 → 6765...
Roundtrip error: ~1.3%
```

---

## Code Structure

| File | Purpose |
|------|---------|
| `src/core/fhe_core.h` | RLWE-based encryption (educational) |
| `src/core/fhe_seal_bridge.h` | SEAL integration for measurement |
| `src/core/cerberus_unified.h` | Combined measurement harness |
| `src/core/true_homomorphic.h` | Polynomial arithmetic layer |

---

## Running Tests

```bash
# Requires: Microsoft SEAL 4.3+, g++

# Addition drift test
g++ -std=c++17 -O2 tests/test_addition_drift.cpp \
    -I . -I /usr/local/include/SEAL-4.3 \
    /usr/local/lib/libseal-4.3.a -pthread -o test_drift && ./test_drift

# UK×UK chain test
g++ -std=c++17 -O2 tests/test_ukuk_chain.cpp \
    -I . -I /usr/local/include/SEAL-4.3 \
    /usr/local/lib/libseal-4.3.a -pthread -o test_ukuk && ./test_ukuk
```

---

## Limitations

- All security relies on standard SEAL BFV
- Noise tracking is empirical, not proven
- Fibonacci moduli are not SEAL-compatible
- No formal security proofs
- Not peer-reviewed

---

## Constants Used

| Constant | Value | Source |
|----------|-------|--------|
| φ | 1.6180339887498948482 | Golden ratio |
| φ⁻¹ | 0.6180339887498948482 | Used in noise model |

---

## Author

Dan Joseph M. Fernandez

---

*"We measured something we don't fully understand. Sharing it in case others can explain it."*

