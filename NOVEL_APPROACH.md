# Why the Golden Ratio Is Used

**Version 2.0**

---

## Purpose

This document explains why this prototype uses the golden ratio (φ) instead of arbitrary parameters. It describes the mathematical properties that motivated this choice.

This is not a claim that φ provides security beyond standard assumptions. It is a record of the observations that led to this approach.

---

## The Core Identity

```
φ = (1 + √5) / 2
ψ = (1 - √5) / 2

φ · ψ = -1
φ + ψ = 1
```

---

## What φ Provides in This Prototype

### 1. Alternating Signs

Multiplying by φ·ψ flips the sign:

```
noise · (φ·ψ) = -noise
```

This alternation is observed in the noise behavior of the FHE implementation. Whether this provides a security benefit beyond RLWE is not claimed.

### 2. Unit Circle Encoding

The iO construction uses e^(iθ) for encoding. All values have |value| = 1. Zero values do not occur.

This is a structural difference from previous iO schemes that used zero-test parameters (and were broken). Whether this is sufficient for security is an open question.

### 3. Uniform Distribution

The golden angle 2π/φ, used as a step size, produces a uniformly distributed sequence. This is tested (1M/1M unique, balance 0.0002).

### 4. Rounding Effect

Lucas numbers satisfy φ^n + ψ^n = integer, with |ψ^n| < 1. The ψ contribution vanishes in rounding.

This is used for a commitment scheme in the prototype. No formal reduction to a hard problem is claimed.

---

## Mathematical Facts Used

| Property | Statement | Relevance |
|----------|-----------|-----------|
| φ·ψ = -1 | Algebraic identity | Alternating signs |
| φ+ψ = 1 | Algebraic identity | Constant sum |
| Hurwitz theorem | \|φ - p/q\| > 1/(√5·q²) | Hard to approximate |
| Weyl criterion | φ^n mod 1 is equidistributed | Uniform distribution |
| Lucas property | φ^n + ψ^n = integer | Rounding effect |

---

## What Is NOT Claimed

- **Not claimed:** That φ provides security beyond RLWE.
- **Not claimed:** That the unit circle encoding is sufficient for iO security.
- **Not claimed:** That the PRNG is cryptographically secure.
- **Not claimed:** That the Lucas commitment has formal binding/hiding proofs.
- **Not claimed:** That these properties are unique to φ.

---

## Why Not Other Constants

This section is explanatory, not a proof of superiority.

| Constant | Observation |
|----------|-------------|
| π | No simple conjugate with product -1 |
| e | Transcendental; no algebraic identity like φ·ψ = -1 |
| √2 | √2 · (-√2) = -2, not -1 |
| φ | φ·ψ = -1, φ+ψ = 1, φ² = φ+1 |

The choice of φ is motivated by the identity φ·ψ = -1. Other constants may have other useful properties.

---

## Relationship to Previous Work

Previous iO candidates (GGH13, CLT13, GGH15) used zero-test parameters and were broken by zeroizing attacks.

This prototype avoids zero-test parameters by using unit circle encoding. This is a structural difference, not a claim of security.

---

## Open Questions

- Does φ provide hardness beyond RLWE?
- Is the unit circle encoding sufficient for formal iO security?
- Is the observed noise damping a consequence of φ·ψ = -1 or of the implementation details?
- Are the performance numbers due to φ or to the small scale of the prototype?

---

## Summary

The golden ratio provides mathematical properties that are useful in this prototype:

1. φ·ψ = -1 → alternating signs
2. Unit circle → no zero values
3. Golden angle → uniform distribution
4. Lucas rounding → commitment effect

These are observations, not security proofs. The approach is a research direction, not a validated scheme.

*φ · ψ = -1*
