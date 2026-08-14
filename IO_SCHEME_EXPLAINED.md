# iO Implementation in This Prototype

**Version 2.0**

---

## What This Document Describes

This document describes the iO implementation in this research prototype. It explains what is implemented and what limitations exist.

This is not a claim of a new iO scheme. It is a description of a construction that uses unit circle encoding to avoid zero-test parameters.

---

## Core Construction

### Golden Orbit Encoding

```
Encoding rule:
  value = e^(iθ)
  TRUE  → θ ∈ (0, π)      (upper half-plane)
  FALSE → θ ∈ (π, 2π)     (lower half-plane)

Property:
  |value| = 1 for all θ
```

### Why Unit Circle

Previous iO schemes (GGH13, CLT13, GGH15) used zero-test parameters. Zeroizing attacks exploited these to recover secrets.

This prototype does not use zero-test parameters. All encoded values have |value| = 1, so zero values do not occur.

This is a structural difference. It is not claimed as a security proof.

---

## Modes

### Truth Table Mode

For functions with few inputs (up to ~4):

- Build a truth table (2^n entries)
- Encode each entry as e^(iθ)
- Evaluate by table lookup

### Circuit Mode

For larger functions:

- Build a NAND circuit (O(n) gates)
- Encode each gate in Golden Orbit
- Evaluate by circuit simulation

| Circuit | Gates | Truth Table Entries |
|---------|-------|---------------------|
| 4-input XOR | 12 | 16 |
| 8-input AND | 14 | 256 |

---

## What Is Tested

| Property | Result | Test File |
|----------|--------|-----------|
| Truth table correctness | 16/16 Boolean functions | `tests/test_all_functions.cpp` |
| Circuit correctness | 4-input XOR 16/16 | `tests/test_circuit_integrated_v2.cpp` |
| KS distance | 0 (100 pairs) | `tests/test_io_stress.cpp` |
| Zero values present | None | `tests/test_adversarial.cpp` |

---

## What Is NOT Claimed

- **No formal indistinguishability proof.** KS=0 is empirical.
- **No claim of security against all attacks.** Only tested against 8 attack classes.
- **No claim that absence of zeros implies security.** This is a structural observation.
- **No comparison to production iO.** There is no production iO. All previous attempts were broken.

---

## Known Limitations

1. Circuit mode supports NAND gates only (universal, but not optimized)
2. No Matrix Branching Programs
3. No formal verification (Coq/Isabelle)
4. Not peer-reviewed
5. Small scale only (2-4 inputs tested)

---

## Relationship to Previous Work

Previous iO candidates (GGH13, CLT13, GGH15) were broken by zeroizing attacks. The common vulnerability was zero-test parameters.

This prototype avoids zero-test parameters entirely by using unit circle encoding. Whether this is sufficient for security is an open question.

---

## Conclusion

This prototype demonstrates an iO construction without zero-test parameters. Tests pass at small scale. This is not a claim of security.

*φ · ψ = -1*
