# Emergent Properties of the Golden Ratio in This Prototype

**Version 2.0**

---

## What This Document Is

This document describes mathematical properties that appear when the golden ratio (φ) is used in the construction of this prototype. These are observed behaviors derived from the identity φ·ψ = -1.

This is not a claim that these properties are unique to φ, nor a claim that they provide security beyond standard assumptions.

---

## The Core Identity

```
φ = (1 + √5) / 2
ψ = (1 - √5) / 2

φ · ψ = -1
```

---

## Observed Properties

### 1. Alternating Signs in Noise

When noise is multiplied by φ·ψ, the sign flips:

```
noise · (φ·ψ) = noise · (-1) = -noise
noise · (φ·ψ)² = noise · (+1) = +noise
```

**What this means in the code:** The NAND gate rescales by 1/golden_plain where golden_plain = Q/φ. This is implemented in `src/fhe/golden_quantum_fhe.h`.

**What is tested:** Noise values remain at golden_plain or 0 after 1000 operations (`tests/test_noise_visual.cpp` in archive).

**What is NOT claimed:** That this provides security beyond RLWE. The damping is a structural property of the construction, not an independent security argument.

---

### 2. Unit Circle Encoding (No Zero Values)

In the Golden Orbit iO, all encoded values are complex exponentials:

```
value = e^(iθ)
|value| = 1 for all θ
```

**What this means:** Zero values do not occur in the encoding. Zeroizing attacks (which broke GGH13/CLT13/GGH15) rely on finding zero values.

**What is tested:** The zero-test resistance check passes (`tests/test_adversarial.cpp`).

**What is NOT claimed:** That this constitutes a formal security proof. It is a structural difference from previous schemes.

---

### 3. Golden Angle Distribution

The golden angle 2π/φ, used as a step size, produces a uniformly distributed sequence:

```
next = (counter · 2π/φ) mod 2π
```

**What is tested:** 1M/1M unique values, balance 0.0002 (`tests/test_golden_prng_inject.cpp`).

**What is NOT claimed:** That this is cryptographically secure randomness. It is a deterministic sequence with good distribution properties.

---

### 4. Lucas Rounding

Lucas numbers satisfy:

```
Lucas(n) = φ^n + ψ^n = integer
|ψ^n| < 1 for n ≥ 1
```

The ψ^n contribution vanishes in rounding, creating a one-way effect:

**What is tested:** 0/100K collisions, tamper detection works (`tests/test_lucas_inject.cpp`).

**What is NOT claimed:** That Lucas-based commitments are as strong as established constructions (e.g., Pedersen commitments). No formal reduction to a hard problem is provided.

---

### 5. Hurwitz Theorem

The golden ratio is the hardest number to approximate by rationals:

```
|φ - p/q| > 1/(√5·q²) for all rational p/q
```

**What this means:** Lattice reduction attacks may be less effective against φ-based constructions.

**What is NOT claimed:** That this provides a security guarantee. This is a structural observation, not a proof.

---

### 6. Weyl Equidistribution

The sequence φ·n mod 1 is equidistributed:

**What is tested:** Uniform distribution across 10 buckets (`tests/test_equidistributed_inject.cpp`).

**What is NOT claimed:** That this replaces established noise distributions (Gaussian, etc.) in production.

---

## Summary Table

| Property | Observed | Tested | Formal Proof |
|----------|----------|--------|--------------|
| Alternating signs | Yes | Yes | No |
| No zero values | Yes | Yes | By construction |
| Uniform distribution | Yes | Yes | Weyl criterion |
| Lucas rounding | Yes | Yes | No |
| Hurwitz bound | Yes | N/A | Proven theorem |
| Equidistribution | Yes | Yes | Weyl criterion |

---

## What This Document Does NOT Claim

- Does NOT claim these properties are unique to φ
- Does NOT claim security beyond RLWE
- Does NOT claim formal verification
- Does NOT claim production readiness

---

*This document is a factual record of observed properties in the prototype.*
