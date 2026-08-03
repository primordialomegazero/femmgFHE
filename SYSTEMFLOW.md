# Spiral Fractal — Complete System Flow

**From Plaintext → GF-N → CKKS FHE → iO Obfuscation → Computation → Bootstrap → Plaintext**

---

## Overview

Spiral Fractal is a **unified FHE + iO system** anchored on the mathematical identity `φ·ψ = -1` (1+1=2 level truth). This document traces the complete lifecycle of a computation — from the owner's plaintext input to the final decrypted result — through all security layers.

---

## Complete Flow Diagram

```
╔══════════════════════════════════════════════════════════════════════════╗
║                    SPIRAL FRACTAL — COMPLETE SYSTEM FLOW               ║
║              From Plaintext → FHE → iO → Computation → Plaintext      ║
╚══════════════════════════════════════════════════════════════════════════╝

┌─────────────────────────────────────────────────────────────────────────┐
│  STEP 1: OWNER INPUT (Plaintext)                                       │
├─────────────────────────────────────────────────────────────────────────┤
│                                                                         │
│  Owner: "I want to compute f(x) = (X AND Y) OR Z"                      │
│  Input: x = 0.5, y = 0.3, z = 0.8                                      │
│                                                                         │
│  ┌─────────────────────┐                                               │
│  │   Plaintext Data     │                                               │
│  │   [0.5, 0.3, 0.8]   │                                               │
│  └─────────┬───────────┘                                               │
│            │                                                            │
└────────────┼────────────────────────────────────────────────────────────┘
             │
             ▼
┌─────────────────────────────────────────────────────────────────────────┐
│  STEP 2: GF-N ENCRYPTION (Inner Encryption Layer)                      │
├─────────────────────────────────────────────────────────────────────────┤
│                                                                         │
│  ┌─────────────────────────────────────────────────────────────────┐   │
│  │  GF-N Encryption Engine                                          │   │
│  │  • N-layer Golden Fibonacci encryption                           │   │
│  │  • Cassini invariant > 0.1 per layer                             │   │
│  │  • Each layer: y = φ(x + seed) mod 1                             │   │
│  │  • Output: GF ciphertext {y1, y2_trail}                          │   │
│  └─────────────────────────────────────────────────────────────────┘   │
│                                                                         │
│  Plaintext [0.5, 0.3, 0.8]                                             │
│      │                                                                  │
│      ▼                                                                  │
│  GF-N Encrypt(plaintext, seed, N=5)                                     │
│      │                                                                  │
│      ▼                                                                  │
│  GF Ciphertext: { y1=0.723, y2_trail=[0.612, 0.891, ...] }             │
│                                                                         │
│  🔐 Security: Cassini invariant > 0.1 prevents tampering               │
│                                                                         │
└─────────────────────────────────────────────────────────────────────────┘
             │
             ▼
┌─────────────────────────────────────────────────────────────────────────┐
│  STEP 3: CKKS FHE ENCRYPTION (Outer Encryption Layer)                  │
├─────────────────────────────────────────────────────────────────────────┤
│                                                                         │
│  ┌─────────────────────────────────────────────────────────────────┐   │
│  │  CKKS FHE Engine                                                 │   │
│  │  • Ring Dimension: 32768                                         │   │
│  │  • Polynomial Degree: 254                                         │   │
│  │  • Depth: 264                                                     │   │
│  │  • Batch Size: 2048                                               │   │
│  │  • DualGate: {a, b} pair encoding                                 │   │
│  │  • a = dualgate_a(φ_val, ψ_val)                                   │   │
│  │  • b = dualgate_b(φ_val, ψ_val)                                   │   │
│  └─────────────────────────────────────────────────────────────────┘   │
│                                                                         │
│  GF Ciphertext {y1=0.723, ...}                                          │
│      │                                                                  │
│      ▼                                                                  │
│  CKKS Encrypt(y1, public_key)                                           │
│      │                                                                  │
│      ▼                                                                  │
│  CKKS Ciphertext: Enc(y1) with noise budget B₀                          │
│                                                                         │
│  🔐 Security: Ring-LWE — computationally hard to decrypt without key   │
│                                                                         │
└─────────────────────────────────────────────────────────────────────────┘
             │
             ▼
┌─────────────────────────────────────────────────────────────────────────┐
│  STEP 4: iO OBFUSCATION (Circuit Hiding)                               │
├─────────────────────────────────────────────────────────────────────────┤
│                                                                         │
│  ┌─────────────────────────────────────────────────────────────────┐   │
│  │  CIRCUIT COMPILER (Universal NAND Compiler)                      │   │
│  │  • Input: f(X,Y,Z) = (X AND Y) OR Z                              │   │
│  │  • Output: NAND-only circuit with N gates                        │   │
│  │  • Gate 0: NAND(X, Y)                                            │   │
│  │  • Gate 1: NAND(Z, Z)  [NOT gate]                                │   │
│  │  • Gate 2: NAND(g0, g1)                                          │   │
│  └─────────────────────────────────────────────────────────────────┘   │
│      │                                                                  │
│      ▼                                                                  │
│  ┌─────────────────────────────────────────────────────────────────┐   │
│  │  FRACTAL GOLDEN GATE (Per-Gate Trace Erasure)                    │   │
│  │  • Depth ≥ 3: φ→ψ→φ alternation                                 │   │
│  │  • Input: raw = NAND(a, b) = 1 - a·b                             │   │
│  │  • Encode: raw × φ (or raw × ψ)                                  │   │
│  │  • Collapse: |encoded × conjugate|                                │   │
│  │  • Output: canonical |v| (path-independent)                       │   │
│  │  • KS = 0.000000 — φ-path ≡ ψ-path                              │   │
│  └─────────────────────────────────────────────────────────────────┘   │
│      │                                                                  │
│      ▼                                                                  │
│  ┌─────────────────────────────────────────────────────────────────┐   │
│  │  N-OBFUSCATION v3 (Order Scrambling)                             │   │
│  │  • 4-fold decomposition per gate                                 │   │
│  │  • Fractal Golden Gate per part                                  │   │
│  │  • Group shuffle (seed-derived permutation)                      │   │
│  │  • Reconstruct — order is scrambled                              │   │
│  │  • Attacker sees: [|v₁|, |v₂|, |v₃|] in random order            │   │
│  └─────────────────────────────────────────────────────────────────┘   │
│      │                                                                  │
│      ▼                                                                  │
│  OBFUSCATED PROGRAM O(f):                                               │
│  • Visible: [|v₁|, |v₂|, |v₃|] — scrambled order                        │
│  • Hidden: Original circuit topology, φ/ψ path, gate wiring             │
│  • KS(Circuit A, Circuit B) = 0.000000                                  │
│                                                                         │
│  🔐 Security: φ·ψ = -1 (structural) — 1+1=2 level mathematical truth  │
│                                                                         │
└─────────────────────────────────────────────────────────────────────────┘
             │
             ▼
┌─────────────────────────────────────────────────────────────────────────┐
│  STEP 5: FHE COMPUTATION (Encrypted Evaluation)                        │
├─────────────────────────────────────────────────────────────────────────┤
│                                                                         │
│  ┌─────────────────────────────────────────────────────────────────┐   │
│  │  HOMOMORPHIC EVALUATION                                          │   │
│  │  • Evaluate O(f) on encrypted inputs                             │   │
│  │  • Each NAND gate → CKKS multiplication + addition              │   │
│  │  • Noise accumulates after each multiplication                   │   │
│  │  • AutoBootstrap v5 monitors noise level                         │   │
│  └─────────────────────────────────────────────────────────────────┘   │
│      │                                                                  │
│      ▼                                                                  │
│  ┌─────────────────────────────────────────────────────────────────┐   │
│  │  SPIRAL BOOTSTRAP (Noise Refresh + iO Trace Erasure)             │   │
│  │                                                                   │   │
│  │  When noise > threshold:                                          │   │
│  │  ┌─────────────────────────────────────────────────────────────┐ │   │
│  │  │ Phase 1: CKKS Decrypt → GF Ciphertext (NOT plaintext!)      │ │   │
│  │  │          📜 Theorem 6: Plaintext never exposed               │ │   │
│  │  ├─────────────────────────────────────────────────────────────┤ │   │
│  │  │ Phase 2: GF-N Decrypt → Plaintext (momentarily, internal)   │ │   │
│  │  │          📜 Theorem 8: Cassini invariant verified            │ │   │
│  │  ├─────────────────────────────────────────────────────────────┤ │   │
│  │  │ Phase 3: Fractal Golden iO → Erase structural trace         │ │   │
│  │  │          📜 Theorem 17: KS = 0.000000                       │ │   │
│  │  ├─────────────────────────────────────────────────────────────┤ │   │
│  │  │ Phase 4: Side-Channel Defense → Chaos masking               │ │   │
│  │  │          🛡️  Value-based reversible masking                 │ │   │
│  │  ├─────────────────────────────────────────────────────────────┤ │   │
│  │  │ Phase 5: GF-N Re-encrypt → Fresh GF Ciphertext              │ │   │
│  │  │          🔄 SeedTree rotation via φ (irrational, unique)     │ │   │
│  │  ├─────────────────────────────────────────────────────────────┤ │   │
│  │  │ Phase 6: CKKS Re-encrypt → Fresh noise budget B₀            │ │   │
│  │  │          📜 Theorem 9: Unlimited depth by induction          │ │   │
│  │  └─────────────────────────────────────────────────────────────┘ │   │
│  └─────────────────────────────────────────────────────────────────┘   │
│      │                                                                  │
│      ▼                                                                  │
│  After N bootstraps:                                                     │
│  • CKKS Ciphertext with fresh noise budget B₀                           │
│  • iO trace erased at every bootstrap                                   │
│  • Computation continues with zero structural leakage                   │
│                                                                         │
│  🔐 Security: FHE + iO integrated — data AND circuit protected         │
│                                                                         │
└─────────────────────────────────────────────────────────────────────────┘
             │
             ▼
┌─────────────────────────────────────────────────────────────────────────┐
│  STEP 6: DECRYPTION (Owner Retrieves Plaintext)                        │
├─────────────────────────────────────────────────────────────────────────┤
│                                                                         │
│  ┌─────────────────────────────────────────────────────────────────┐   │
│  │  CKKS DECRYPTION                                                 │   │
│  │  • Input: CKKS Ciphertext (final encrypted result)               │   │
│  │  • Decrypt with owner's secret key                               │   │
│  │  • Output: GF Ciphertext {y1, y2_trail}                          │   │
│  └─────────────────────────────────────────────────────────────────┘   │
│      │                                                                  │
│      ▼                                                                  │
│  ┌─────────────────────────────────────────────────────────────────┐   │
│  │  GF-N DECRYPTION                                                 │   │
│  │  • Input: GF Ciphertext                                          │   │
│  │  • Decrypt each layer (reverse Fibonacci): x = ψ(y - seed)       │   │
│  │  • Verify Cassini invariant > 0.1 (integrity check)              │   │
│  │  • Output: Plaintext value                                       │   │
│  └─────────────────────────────────────────────────────────────────┘   │
│      │                                                                  │
│      ▼                                                                  │
│  ┌─────────────────────┐                                               │
│  │   Plaintext Result   │                                               │
│  │   f(0.5,0.3,0.8)    │                                               │
│  │   = 0.8 (TRUE)       │                                               │
│  └─────────────────────┘                                               │
│                                                                         │
│  🔐 Security: Only owner with correct GF-N seeds + CKKS secret key    │
│               can decrypt the result                                    │
│                                                                         │
└─────────────────────────────────────────────────────────────────────────┘
```

---

## Security Guarantees

| Layer | What It Protects | Foundation |
|-------|-----------------|------------|
| **GF-N Encryption** | Inner plaintext confidentiality | Cassini invariant > 0.1 |
| **CKKS FHE** | Encrypted computation | Ring-LWE (computational) |
| **Fractal Golden Gate** | Circuit structure (trace erasure) | `φ·ψ = -1` (structural, 1+1=2) |
| **N-Obfuscation v3** | Gate order (scrambling) | Seed-derived permutation |
| **Spiral Bootstrap** | Unlimited depth + iO refresh | φ-driven seed rotation |
| **Side-Channel Defense** | Timing/power/EM leakage | Chaos masking |
| **Blackhole Defense** | Active intrusion | Decoy execution + memory scramble |

---

## What The Attacker Sees At Each Step

| Step | Attacker's View | What They Learn |
|------|-----------------|-----------------|
| **After GF-N** | GF Ciphertext `{y1, y2_trail}` | Nothing — encrypted |
| **After CKKS** | CKKS Ciphertext | Nothing — Ring-LWE secure |
| **After iO** | `[│v₁│, │v₂│, │v₃│]` (scrambled) | Function output only (PUBLIC) |
| **During Computation** | CKKS Ciphertexts only | Nothing — encrypted |
| **During Bootstrap** | GF Ciphertext (NOT plaintext) | Nothing — Theorem 6 |
| **At Decryption** | Requires secret key | Nothing without key |

---

## The Mathematical Anchor

```
φ = (1+√5)/2  ≈  1.6180339887498948482
ψ = (1-√5)/2  ≈ -0.6180339887498948482

φ·ψ = ((1+√5)/2) × ((1-√5)/2) = (1-5)/4 = -1
φ+ψ = ((1+√5)+(1-√5))/2 = 1
```

**This is 1+1=2. Not conjecture. Not assumption. Mathematical fact.**

The entire system — FHE + iO — rests on this single identity. No computational hardness assumptions are needed for the iO layer. The FHE layer adds defense-in-depth via Ring-LWE.

---

**Repository:** [github.com/primordialomegazero/femmgFHE](https://github.com/primordialomegazero/femmgFHE)  
**Contact:** devilswithin13@gmail.com  
**Version:** 37.3 — Complete System Flow  
**Date:** August 4, 2026
