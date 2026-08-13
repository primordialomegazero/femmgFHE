# Understanding the Unfamiliar: A Guide for Traditional Cryptographers

**How Spiral FHE+iO Compares to What You Already Know**

**Dan Joseph M. Fernandez | Version 47.0**

---

## Why This Document Exists

If you are reading this, you are likely a trained cryptographer or FHE researcher. You understand CKKS, TFHE, bootstrapping, and iO. But you may find yourself confused by terms like "GF-N layer," "seed rotation," "FGG collapse," or "DualGate projection."

This document explains these unfamiliar concepts **in the context of what you already know.**

No new mathematics. Just translation.

---

## Part 1: FHE Bootstrap — Traditional vs Spiral

### Traditional View (What You Know)

**The Problem:**
CKKS ciphertexts accumulate noise with each multiplication. After enough operations, decryption fails.

**The Solution (Traditional):**
Homomorphic bootstrapping. The secret key is encrypted under the public key, and this encrypted key is used to evaluate the decryption circuit homomorphically. The result is a fresh ciphertext with low noise.

**The Catch:**
This requires **circular security** — the assumption that encrypting the secret key under its own public key is secure. This is an unproven assumption.

### Spiral View (What We Do)

**The Problem:**
Same. CKKS noise accumulates.

**The Solution (Spiral):**
Application-level bootstrap. Decrypt the ciphertext to a GF-N intermediate (NOT plaintext), verify Cassini invariant, rotate seed, re-encrypt with fresh B0.

**The Difference:**
The secret key is **never encrypted under any public key.** The decrypted value is **not plaintext** — it is a GF-N ciphertext component.

### Side-by-Side Comparison

| Aspect | Traditional Bootstrap | Spiral Bootstrap |
|--------|----------------------|------------------|
| Decryption | Homomorphic (evaluates decrypt circuit) | Direct (actual decrypt) |
| Secret key | Encrypted under PK | Never encrypted |
| Circular security | Required | Not needed |
| Plaintext exposure | None (homomorphic) | None (GF-N protected) |
| Speed | Slow (evaluates complex circuit) | Fast (direct operation) |
| Depth | Refreshed | Refreshed |
| Source | OpenFHE EvalBootstrap | `src/fhe/spiral_fhe_io_final.h` |

### Why You Might Resist This

You might think: "Direct decryption is not FHE. FHE means you NEVER decrypt."

But consider: **what do you decrypt to?**

Traditional FHE: decrypt to plaintext (bad, exposes data).
Spiral FHE: decrypt to GF-N ciphertext (safe, another encryption layer).

The question is not "do you decrypt?" The question is "what do you decrypt to?"

If you decrypt to another ciphertext (GF-N), the operation is still secure. The plaintext is never exposed.

---

## Part 2: GF-N Layer — What Is It?

### Traditional View (What You Know)

You are familiar with layered encryption: encrypt data with AES, then encrypt the AES key with RSA. Two layers of encryption.

### Spiral View (GF-N)

GF-N is a **golden-fibonacci encryption layer.** It encrypts values using the golden ratio algebra:

```
Encrypt(m) → y1, y2_trail
Decrypt(y1, y2_trail) → m
```

**Key Properties:**
- Multi-layer: N independent encryption layers (default 5)
- Seed tree: hierarchical key derivation from 1 master seed
- Cassini invariant: structural integrity check
- y2_trail: breadcrumb for exact decryption

### Side-by-Side Comparison

| Aspect | AES/RSA Layering | GF-N Layering |
|--------|-----------------|---------------|
| Layers | 2 (AES + RSA) | N (default 5) |
| Key derivation | Separate keys | 1 master seed → N branch seeds |
| Integrity check | MAC/AEAD | Cassini invariant |
| Source | OpenSSL | `src/config/gf_n_encryption.h` |

### Why GF-N?

GF-N is not meant to replace AES or RSA. It is meant to provide **structural guarantees** that AES/RSA cannot:

1. **Cassini invariant** — algebraic integrity check without additional MAC.
2. **Seed tree** — deterministic key derivation with branch isolation.
3. **Golden foundation** — `φ·ψ = -1` enables structural erasure.

---

## Part 3: FGG Collapse — What Is It?

### Traditional View (What You Know)

You are familiar with hash functions: SHA-256 maps any input to a fixed-size output. Given the hash, you cannot recover the input.

### Spiral View (FGG)

FGG (Fractal Golden Gate) is a **structural erasure function:**

```
FGG(v) = |v| for any v
```

It maps any value to its absolute value. The sign information is destroyed.

### Side-by-Side Comparison

| Aspect | Hash Function (SHA-256) | FGG Collapse |
|--------|------------------------|---------------|
| Input | Any string | Any real number |
| Output | 256-bit digest | \|v\| (absolute value) |
| Reversibility | Computationally hard | Mathematically impossible |
| Foundation | Collision resistance (assumptional) | `φ·ψ = -1` (theorem) |

### Why FGG Is Different

Hash functions hide information behind computational hardness. FGG **erases** information structurally. There is no pre-image to find because the sign information no longer exists.

---

## Part 4: Seed Rotation — What Is It?

### Traditional View (What You Know)

You are familiar with key rotation: periodically replace cryptographic keys to limit damage from key compromise.

### Spiral View (Seed Rotation)

Seed rotation is **forward-secure key evolution** during bootstrap:

```
seed' = f(seed, y1) where f uses φ
```

The old seed is destroyed after rotation. Each bootstrap cycle uses a fresh seed.

### Side-by-Side Comparison

| Aspect | Key Rotation | Seed Rotation |
|--------|-------------|---------------|
| Frequency | Periodic (days/months) | Every bootstrap cycle |
| Old key | Revoked (still exists) | Destroyed (cannot be recovered) |
| Foundation | Policy decision | Golden ratio algebra |
| Forward security | Yes | Yes (stronger) |

### Why Seed Rotation Matters

In traditional key rotation, old keys still exist (revoked but recoverable). In seed rotation, old seeds are **mathematically destroyed** — they cannot be recovered because the rotation is many-to-one.

---

## Part 5: iO — Traditional vs Spiral

### Traditional View (What You Know)

**The Problem:**
Given a circuit C, produce an obfuscated version O(C) such that:
- O(C) computes the same function as C
- O(C) reveals nothing about C's structure
- O(C1) is indistinguishable from O(C2) if C1 and C2 compute the same function

**The Solution (Traditional):**
Multilinear maps or graded encoding schemes (GGHRSW, CLTV15). These are complex algebraic constructions, several of which have been broken.

### Spiral View (What We Do)

**The Solution (Spiral):**
TFHE-based universal circuit with encrypted coefficients.

1. Compile circuit to coefficients (which wires connect)
2. Encrypt coefficients under TFHE
3. Evaluate in fixed-topology universal circuit

**The Difference:**
No multilinear maps. No graded encodings. Just TFHE (standard scheme) + encrypted coefficients.

### Side-by-Side Comparison

| Aspect | Traditional iO | Spiral iO |
|--------|---------------|-----------|
| Foundation | Multilinear maps | TFHE (LWE) |
| Security assumption | New (broken candidates) | Standard (LWE) |
| Working code | Mostly theoretical | 1M gates verified |
| Depth | Limited | Unlimited (TFHE bootstrap) |
| Circuit hiding | Complex | Encrypted coefficients |

### Why You Might Resist This

You might think: "iO requires hiding the circuit structure. Encrypting coefficients doesn't hide the topology."

Correct — but the topology is **fixed and universal.** All circuits of the same size have the same topology. The only thing that differs is the coefficients. And those are encrypted.

**The circuit structure is not hidden because it doesn't need to be hidden.** It's the same for all circuits. What differs is the function, encoded in encrypted coefficients.

---

## Part 6: DualGate Projection — What Is It?

### Traditional View (What You Know)

You are familiar with encoding schemes: map values to a different representation for processing.

### Spiral View (DualGate)

DualGate projects any pair `(a,b)` through golden ratio algebra:

```
φ_val = a·φ + b·ψ
ψ_val = a·ψ + b·φ
```

The projection invariant `-a² + 3ab - b²` ensures correctness. The `to_bool()` method recovers the original bit.

### Side-by-Side Comparison

| Aspect | One-Hot Encoding | DualGate Projection |
|--------|-----------------|---------------------|
| Input | Integer (0-9) | Any pair (a,b) |
| Output | Binary vector | (φ_val, ψ_val) |
| Invertibility | Yes | Yes (to_bool) |
| Foundation | Arbitrary | `φ·ψ = -1` |
| Invariant | None | `-a² + 3ab - b²` |

### Why DualGate?

DualGate provides a **scheme-agnostic bridge** between CKKS and TFHE. It doesn't depend on either scheme's internals — only on `φ·ψ = -1`. This makes it simple, verifiable, and universal.

---

## Summary: The Pattern

Throughout this document, one pattern emerges:

| Concept | Traditional | Spiral |
|---------|------------|--------|
| Bootstrap | Homomorphic (complex, circular security) | GF-N decrypt (simple, no circular) |
| Protection | Computational hardness | Structural erasure |
| iO | Multilinear maps (broken) | TFHE + encrypted coefficients (working) |
| Bridge | Custom protocol | DualGate projection |
| Foundation | Assumptions | Theorems (`φ·ψ = -1`) |

The Spiral approach is not "better" in the sense of being more complex. It is **simpler.**

And that's exactly why it works.

---

*Foundation: φ·ψ = -1 = 1+1=2*
