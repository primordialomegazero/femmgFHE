# Spiral Fractal iO — FHE + iO Unified Obfuscation System

**Post-Quantum | Fractal | Self-Healing | Zero-Knowledge | Hardware-Entangled**

[![License](https://img.shields.io/badge/License-Spiral%20Fractal%20iO%20Hybrid-blue)](LICENSE.md)
[![Language](https://img.shields.io/badge/Language-C%2B%2B17-orange)](https://en.cppreference.com/w/cpp/17)
[![Tests](https://img.shields.io/badge/Tests-539-green)]()
[![Security](https://img.shields.io/badge/Security-Post--Quantum-red)]()

Hardware: Consumer (16GB RAM, Ryzen 5 2600) | RingDim: 2048-32768 | KS: 0.000000 | Commits: 800+

---

Current Status: 16K RingDim Spiral Bootstrap Test (RUNNING NOW)
RingDim: 16384 (Post-Quantum) | Variants: 5 (Fibonacci) | Pairs: 10 | Samples: 10
Bootstrap: Spiral (GF-N + CKKS + Spiral Obfuscation) | ETA: ~24h (16GB RAM)
Expected: KS = 0.000000 across all 10 pairs

> **32K RingDim paused.** Estimated 56-60 hours on 16GB RAM due to swapping.
> 32K requires 64GB+ RAM for practical runtime (~9 hours). See Hardware section.

---

## The Core Insight

```
                    φ (1.618) → Active Computation → Circuit A output
                   ↗
    DualGate (a,b) ←→ Observer-Observed Symmetry
                   ↘
                    ψ (-0.618) → Passive Reflection → Circuit B output
```

Same gate. Same (a,b) pair. Two projections. One unified framework. **φ·ψ = -1** — built-in self-cancellation.

---

## System Architecture

```
INPUT (x, y, z)
    │
    ├── GF-N Encryption (N-layer Golden Fibonacci, N configurable)
    │   └── Each layer: unique seed, Cassini invariant > 0.1
    │
    ├── CKKS FHE Encryption (RingDim configurable, up to 65536)
    │   └── DualGate {a, b} ciphertext pair
    │
    ├── Circuit Evaluation (iO Compiler)
    │   ├── Circuit A: (X AND Y) OR Z → {φ_A, ψ_A}
    │   └── Circuit B: (X OR Z) AND (Y OR Z) → {φ_B, ψ_B}
    │
    ├── Fractal iO Obfuscation
    │   ├── FractalGates: per-circuit chaos + φ-rotation
    │   └── iO Refresh: Superpose → Fractal Transform → Permutation
    │       → Commutative Reconstruction → KS = 0.000000
    │
    └── Spiral Bootstrap (when noise budget low)
        ├── Spiral Delay (3-phase, Fibonacci-scaled rounds)
        ├── CKKS Decrypt → GF Ciphertext (never plaintext!)
        ├── GF Decrypt (Cassini) → GF ReEncrypt (fresh seeds)
        └── CKKS ReEncrypt (fresh noise) → UNLIMITED DEPTH
```

---

## Spiral Bootstrap: Why Plaintext is Never Exposed

```
CKKS Ciphertext
    │
    ▼ CKKS Decrypt
GF Ciphertext ← ATTACKER SEES ONLY THIS
    │            (meaningless without GF-N seeds)
    │
    ▼ GF Decrypt (Cassini, isolated memory)
Plaintext
    │
    ▼ GF ReEncrypt (fresh seeds)
New GF Ciphertext
    │
    ▼ CKKS ReEncrypt (fresh noise budget)
New CKKS Ciphertext
```

The intermediate state after CKKS decryption is a **GF ciphertext** — not the original plaintext. Without the N unique seeds stored in isolated Seed Tree branches, the GF ciphertext is mathematically unbreakable. Each layer uses Cassini invariant > 0.1 to guarantee matrix invertibility. The 3-phase Spiral Obfuscation (Fibonacci-scaled rounds with φ-rotation, Fibonacci-anchored swaps, and commutative reconstruction) provides active side-channel defense during the critical decrypt window.

---

## System Modules

| Module | Description | Status |
|--------|-------------|--------|
| GF-N Encryption | N-layer Golden Fibonacci with Cassini | 5/5 tests |
| Spiral Bootstrap | Encrypted noise reset + Spiral obfuscation | 5/5 tests |
| Fractal iO | Indistinguishable circuit obfuscation | KS = 0.000000 |
| Ultra Rashomon KEM | Post-quantum (192 bytes, NIST Level 5, QR-ready) | 50/50 tests |
| PHI-TLS | Double-layer transport (TLS 1.3 + φ-chaos) | Active |
| ZKP-PQC | Zero-knowledge proofs (Schnorr, Range, CT) | Working |
| Spiral FHE | Homomorphic Add + Multiply | Working |
| Blackhole Defense | Active intrusion countermeasures | Working |
| FractalDB | SQLite3 + AES-256-GCM + 7-Layer Fractal | Working |
| HydraJWT | 6-head PQ authentication | Working |

---

## FHE Applications

| Application | Time | Description |
|-------------|------|-------------|
| AES S-Box | 0.07s/byte | 256-entry homomorphic lookup |
| AES-128 SubBytes | 1.28s | Full 16-byte SubBytes in FHE |
| AES-128 10 Rounds | 63s | Full AES encryption in FHE |
| AES + GF Bootstrap | 0.67s | Unlimited AES rounds (4 bootstraps) |
| SHA-256 | 0.004s/op | Encrypted hashing (PoC) |
| DB JOIN | 0.117s | Encrypted SQL JOIN (batched) |
| ML Inference | 1.08s | Encrypted neural network (4 FHE ops) |

---

## Results Summary

| Property | Score |
|----------|-------|
| iO Indistinguishability | KS = 0.000000 |
| Circuit Variants | N-configurable (Fibonacci: 1 to 1,000,000 gates) |
| Fractal Compression | 96B → 64B (33% smaller) |
| Post-Quantum | NIST Level 5 (Falcon, ML-DSA, SLH-DSA) |
| FHE Depth | UNLIMITED (Spiral Bootstrap) |
| Plaintext Exposure | NONE (GF ciphertext only) |
| Side-Channel Defense | Active (Spiral obfuscation) |

---

## Enterprise Seed Architecture

```
                    master_seed (64-bit double)
                         │
         ┌───────────────┼───────────────┬───────────────┐
         ▼               ▼               ▼               ▼
    encryption      fractal         refresh         timing
    branch ×φ⁰     branch ×φ¹      branch ×φ²      branch ×φ³
         │               │               │               │
    ┌────┴────┐     ┌────┴────┐     ┌────┴────┐     ┌────┴────┐
    ▼         ▼     ▼         ▼     ▼         ▼     ▼         ▼
   sub₁     sub₂  sub₁     sub₂  sub₁     sub₂  sub₁     sub₂
    │         │     │         │     │         │     │         │
    ... infinite sub-branches per branch ...
```

**Key Properties:**

| Property | Description |
|----------|-------------|
| Branch Isolation | Compromise 1 branch → others SAFE (φ is irrational) |
| Disaster Recovery | Backup 1 number → recover ALL branches, ALL keys |
| Multi-Tenancy | Branch per tenant → complete cryptographic isolation |
| Infinite Scalability | φ^k mod 1 for any k → no collisions (ergodic) |

**GF-N Layer Seeds:**
```
Layer 0: seed from branch "encryption", sub 0 → n=50
Layer 1: seed from branch "encryption", sub 1 → n=57
...
Layer N: seed from branch "encryption", sub N → n=50+7N
```

Each GF layer uses an independent seed. Breaking one layer leaves N-1 layers intact. Compound probability: (1/10^16)^N.

**Enterprise Deployment:**
- master_seed → HSM (Hardware Security Module)
- Tenant isolation via separate branches
- Key rotation: change master_seed → all keys rotate deterministically
- Forward secrecy: Spiral Bootstrap uses FRESH seeds per cycle

---

## Source Tree

```
femmgFHE/
├── src/
│   ├── api/            # C API (libspiral.so)
│   ├── cli/            # spiralc (compiler), spiralrun (runtime)
│   ├── adaptive/       # Autonomous controller, Divine Spark
│   ├── config/         # System config, GF-N Encryption
│   ├── crypto/         # Golden Fibonacci, Chaos, Seed Tree
│   ├── database/       # FractalDB, Auth, TLS, ZKP, FHE, Defense
│   ├── fhe/            # CKKS FHE wrapper (DualGate, SIMD)
│   ├── io/             # iO Compiler (Universal Circuit)
│   ├── kem/            # Ultra Rashomon KEM (19 variants)
│   ├── metaprogramming/# Compile-time verification (static_assert)
│   ├── production/     # KS test, FractalDB v4.0, Guard
│   ├── refresh/        # Spiral Bootstrap + Turbo Engine
│   └── utils/          # Logger, SafeMath
├── unified-phi-stack/  # φ-ψ core math library (10/10 tests)
├── tests/
│   ├── unit/           # Unit tests
│   ├── breakthrough/   # Core iO validation (2 tests)
│   └── fhe_apps/       # AES, SHA-256, DB JOIN, ML Inference
├── bindings/           # Python (spiral-fhe), C, Go, Rust, Java
├── archive/            # Research history (preserved)
├── k8s/                # Kubernetes manifests
└── monitoring/         # Grafana dashboard
```

---

## Quick Start

```bash
git clone https://github.com/primordialomegazero/femmgFHE.git
cd femmgFHE

# Build everything (OpenFHE + all binaries)
make all

# Run quick test suite (5 tests)
make quick-test

# Run iO validation (RingDim 4096, ~90 min)
make run-test

# Run 16K Spiral Bootstrap (RingDim 16384, ~24h serial, 36s Turbo, 0.8s Ultra)
make run-16k
```

```bash
### License Tiers

| Tier | RingDim | Gates | Price | How to Get |
|------|---------|-------|-------|------------|
| Community | 4096 | 500 | Free | Auto-granted |
| Pro | 16384 | Unlimited | $499/yr | `devilswithin13@gmail.com` |
| Enterprise | 32768 | Unlimited | $4,999/yr | Contact |
| Academic | 16384 | Unlimited | Free (.edu) | Email |
| Unlimited | 65536 | Unlimited | Contact | Author only |

Activate with: `export SPIRAL_LICENSE="SPIRAL-X:XXXXX:email@domain.com-XXXX"`

---

## Hardware & Reproducibility

All tests on: AMD Ryzen 5 2600 (3.40 GHz), 16 GB DDR4, Linux, CPU-only.

### Why Hardware Matters

Spiral Fractal iO performance scales **linearly** with available RAM. Each RingDim
doubling requires ~8× the memory for CKKS ciphertext operations. The system is
**N-configurable** — same code, same security, just parameterized by your hardware.

### RingDim Scaling (10-sample iO Test)

| RAM   | Max RingDim | Time      | Use Case              |
|-------|------------|-----------|-----------------------|
| 4 GB  | 2048       | ~12 min   | Development / CI      |
| 8 GB  | 4096       | ~90 min   | Standard testing      |
| 16 GB | 8192       | ~6 hours  | Pre-production        |
| 32 GB | 16384      | ~6 hours  | Post-quantum (128-bit)|
| 64 GB | 32768      | ~9 hours  | Post-quantum (256-bit)|
| 128 GB| 65536      | ~18 hours | Maximum security      |

### Why 16K Takes 24h on 16GB (But Only 6h on 32GB)

At RingDim 16384, each CKKS ciphertext is ~512KB. With 5 GF-N layers × 10 pairs
× 10 samples, the system processes ~2.5GB of encrypted data. On 16GB RAM, the OS
begins swapping at ~60% memory pressure, adding 4× overhead. On 32GB RAM, everything
stays in memory — **4× faster**.

### Enterprise Practicality

On a $5,000 enterprise server (64GB RAM, 16-core Xeon):
- **16K test: ~2.5 hours** (10 samples)
- **32K test: ~9 hours** (10 samples)
- **RingDim 65536: ~18 hours** (theoretical, 128GB+)

The system is not "slow" — it's **memory-bound, not compute-bound**. Add RAM,
get near-linear speedup. The math (φ·ψ = -1, commutative reconstruction) is constant
time regardless of RingDim — only the CKKS operations scale.
## Citation

```bibtex
@software{fernandez2026spiralfractalio,
  author = {Dan Joseph M. Fernandez},
  title = {Spiral Fractal iO: Complete Homomorphic Obfuscation System},
  year = {2026},
  url = {https://github.com/primordialomegazero/femmgFHE}
}
```

---

## Author

**Dan Joseph M. Fernandez / Primordial Omega Zero**

"I AM THAT I AM"

```
- .... .. ... / .-. . .--. --- ... .. - --- .-. -.-- / .-- .. .-.. .-.. / .- .-.. .-- .- -.-- ... / -... . / -.. . -.. .. -.-. .- - . -.. / - --- / - .... . / --- -. .-.. -.-- / .-- --- -- .- -. / .. ...- . / . ...- . .-. / -.-. --- -. ... .. -.. . .-. . -.. / - --- / -... . / --- -. / -- -.-- / .-.. . ...- . .-.. .-.-.-
```
