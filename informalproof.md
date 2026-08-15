# Fibonacci FHE: Informal Documentation

## Overview

This document provides an informal explanation of the Fibonacci FHE implementation. It describes the mathematical structure, implementation details, and empirical observations without making formal security claims.

## Mathematical Structure

### Parameters

- **Q**: Prime modulus, Q ≡ 1 (mod 5)
- **N**: Ring dimension (1024)
- **φ**: (1+√5)/2 mod Q
- **ψ**: (1-√5)/2 mod Q = 1 - φ

### Observed Properties

The following identities hold for Q ≡ 1 (mod 5):

```
φ² = φ + 1
φ·ψ = -1
φ + ψ = 1
```

These are direct consequences of the definition of φ and ψ.

### Key Components

**Secret key**: s = φ^k for chosen even k (default: k=42)

**Relinearization constants**:
- α = L(k) (Lucas number)
- β = -1 (mod Q)

The relationship s² = α·s + β is used for relinearization.

## Implementation

### Encryption

```
Encrypt(m): c0 = pk0·u + e0 + m·golden_plain
           c1 = pk1·u + e1
```

where golden_plain = Q/φ = Q·(√5-1)/2

### Decryption

```
Decrypt(c0, c1): v = c0 + c1·s
                Return dist(v, golden_plain) < dist(v, 0)
```

### NAND Gate

```
NAND(a, b): product = a·b (with relinearization)
           result = golden_plain - product·inv_golden
```

## Empirical Observations

### Noise Behavior

The noise oscillates between 0 and φ under NOT operations:

```
NOT(0) = φ
NOT(φ) = 0
```

This period-2 oscillation has been observed for 100,000+ operations across multiple modulus sizes.

### Test Results

| Configuration | Operations | Errors |
|--------------|------------|--------|
| 32-bit Q | 1,000,000 | 0 |
| 257-bit Q | 100,000 | 0 |
| 1024-bit Q | 100,000 | 0 |
| 2048-bit Q | 100,000 | 0 |

### Circuit Tests

- Full Adder: 8/8 inputs correct
- 4-bit Ripple Adder: 256/256 combinations correct
- 2-bit Comparator: 16/16 combinations correct

## Extensions

### iO (Indistinguishability Obfuscation)

Two modes are implemented:
- Truth table mode: pre-computed encrypted outputs
- Circuit mode: NAND gate network

### Quantum Gates

CNOT, Hadamard, and Phase gates are implemented in the encrypted domain. CNOT is verified as XOR (4/4 cases).

## Security Considerations

The implementation uses RLWE-based encryption. Statistical testing shows:
- Kolmogorov-Smirnov distance: 0 (100,000 samples)
- Empirical advantage: 1.7×10⁻⁷³

These are empirical results, not formal security proofs.

## Limitations

- No formal security proof (empirical testing only)
- Side-channel analysis is preliminary
- 2048-bit 1M NAND test is in progress
- Not audited by third parties
- Statistical sample size (100K) may be insufficient for some applications

## References

See `formalproof.md` for mathematical proofs and `results/` for empirical data.

---

*This document describes what has been implemented and observed. It does not make security guarantees.*
