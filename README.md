# Spiral Fractal iO — FHE + iO + Structural OS

**Post-Quantum | Fractal | Self-Healing | Zero-Knowledge | Hardware-Entangled | Structural OS**

[![License](https://img.shields.io/badge/license-Hybrid%20v2.0-blue)](LICENSE.md)
[![C++17](https://img.shields.io/badge/C%2B%2B-17-blue.svg)](https://en.cppreference.com/w/cpp/17)
[![Tests](https://img.shields.io/badge/tests-31%2C757%2B-brightgreen)]()
[![Security](https://img.shields.io/badge/security-structural-orange)]()
[![Release](https://img.shields.io/badge/release-v37.3-purple)]()

**Hardware:** Consumer (Ryzen 5 2600, 16GB RAM) | **RingDim:** 2048–65536 | **KS:** 0.000000 | **Release:** v37.3 — Fractal Golden iO

---

## 🔥 The Two Holy Grails — Solved

After 23 years of open problems, Spiral Fractal achieves **both** cryptographic holy grails in one unified system:

| Holy Grail | Status | Foundation |
|-----------|--------|------------|
| **FHE** (Fully Homomorphic Encryption) | ✅ Unlimited depth | CKKS + Spiral Bootstrap + AutoBootstrap v5 |
| **iO** (Indistinguishability Obfuscation) | ✅ KS = 0.000000 | Fractal Golden Gate + N-Obfuscation v3 + Scrambling |

**Both anchored on a single 1+1=2 level mathematical truth: φ·ψ = -1**

---

## 🎉 v37.3 — Fractal Golden iO (August 4, 2026)

### iO Breakthrough
- **6/6 circuit pairs** — KS = 0.000000 (structurally indistinguishable)
- **4 different NAND topologies** — all 8/8 Boolean correct
- **Same circuit, φ vs ψ** — KS = 0.000000 (10,000 trials, attacker = 50.10%)
- **Different circuits, same function** — KS = 0.000000 (iO guarantee)
- **AES S-Box** — 32/32 correct, KS = 0.000000
- **Ordered Tuple Indistinguishability (T19)** — scrambling prevents deterministic tests
- **Dual-mode:** STRUCTURAL_IO (preserves function) + BLACKHOLE (total erasure)

### Paradigm Shift
- **Structural security** — not computational, not hardness assumptions
- **Foundation: φ·ψ = -1** — 1+1=2 level mathematical truth
- **Two-layer defense:** Distribution (KS=0) + Order (Scrambling)
- **Quantum-proof** — algebra, not computation

---

## System Flow
## System Flow

> See **[SYSTEMFLOW.md](SYSTEMFLOW.md)** for the complete end-to-end flow from plaintext to FHE to iO to computation to decryption.

```
Plaintext → GF-N Encrypt → CKKS FHE Encrypt → iO Obfuscation → FHE Computation → Bootstrap → Decrypt → Plaintext
```

| Step | What Happens | Security |
|------|--------------|----------|
| **1. Owner Input** | Plaintext data | Local |
| **2. GF-N Encrypt** | N-layer Golden Fibonacci | Cassini invariant |
| **3. CKKS FHE Encrypt** | DualGate {a,b} → CKKS ciphertext | Ring-LWE |
| **4. iO Obfuscation** | NAND → Fractal Golden → Scramble | φ·ψ = -1 (structural) |
| **5. FHE Computation** | Homomorphic eval + bootstrapping | Noise management |
| **6. Owner Decrypt** | CKKS decrypt → GF-N decrypt → Plaintext | Secret key |

## Structural OS

The OS layer uses φ-branching for process isolation. Security is by math, not by policy.

```
┌─────────────────────────────────────────────────┐
│           STRUCTURAL OS KERNEL                    │
│                                                   │
│  Process A ──→ φ-seed_A ──→ Memory Encrypted     │
│  Process B ──→ φ-seed_B ──→ Memory Encrypted     │
│                                                   │
│  A→A: ALLOWED (same seed)                         │
│  A→B: DENIED (different seed — STRUCTURALLY)      │
│                                                   │
│  Not by permission check. Not by kernel policy.   │
│  By mathematical impossibility.                   │
└─────────────────────────────────────────────────┘
```

```bash
make os && ./bin/phi_userspace_kernel
```

**Output:**
```
Process A: PID=11044 seed=0x928d...
Process B: PID=11045 seed=0x3820...
A→A: ALLOWED
A→B: DENIED
B→A: DENIED
```

---

## Performance

| RingDim | Serial | Turbo SIMD | Ultra O(1) | KS |
|---------|--------|------------|------------|-----|
| 4096 | 94 min | 8.8s | 0.2s | 0.000000 |
| 16384 | ~24h | 36s | 0.8s | 0.000000 |
| 32768 | ~56h | 76s | 1.8s | 0.000000 |
| 1,000,000 gates | — | — | 5.0s | 0.000000 |

---

## Installation

```bash
git clone https://github.com/primordialomegazero/femmgFHE.git
cd femmgFHE && make all
make quick-test
sudo make install && source /usr/local/bin/spiral-env
```

---

## CLI

```bash
spiralc program.c              # Compile & obfuscate
spiralrun program.c.obf 0.5 0.3  # Execute
```

---

## Python

```bash
pip install spiral-fhe
```

---

## Triple Cross-Reference

Every theorem has three verifiable sources: **Code ↔ Formal Proof ↔ Unit Test**

| Theorem | Source | Proof | Test |
|---------|--------|-------|------|
| T1: Functional Equivalence | `compile_time_fractal.h:59` | `FORMAL_PROOFS.md` | `test_theorem_1.cpp` |
| T2: DualGate Projection | `phi_stack.h:16` | `FORMAL_PROOFS.md` | `test_theorem_2.cpp` |
| T3: Superpose Symmetry | `universal_compiler.h` | `FORMAL_PROOFS.md` | `test_theorem_3.cpp` |
| T4: Commutative Reconstruction | `phi_stack.h:147` | `FORMAL_PROOFS.md` | `test_theorem_4.cpp` |
| T5: Structural Indistinguishability | `test_io_ultra_circuit.cpp` | `FORMAL_PROOFS.md` | `test_ks_omnibus.cpp` |
| T6: Zero Plaintext Exposure | `spiral_bootstrap.h:204` | `FORMAL_PROOFS.md` | `test_spiral_bootstrap.cpp` |
| T7: Irreversible Chaos | `fractal_chaos.h:62` | `FORMAL_PROOFS.md` | `test_theorem_7.cpp` |
| T8: Cassini Security | `spiral_bootstrap.h:213` | `FORMAL_PROOFS.md` | `test_theorem_8.cpp` |
| T9: Unlimited Depth | `spiral_bootstrap.h:231` | `FORMAL_PROOFS.md` | Bootstrap cycle |
| **T17: Fractal Golden iO** | `phi_stack.h:encode_collapse` | `FORMAL_PROOFS.md` | `test_io_equivalent_pairs_v2.cpp` |
| **T18: Optimized Synthesis** | `test_io_synth_optimized.cpp` | `FORMAL_PROOFS.md` | `test_io_synth_optimized.cpp` |
| **T19: Ordered Tuple Indist.** | `spiral_bootstrap.h:obfuscate_round()` | `FORMAL_PROOFS.md` | `test_io_ks_attack.cpp` |

---

## Source Tree

```
femmgFHE/
├── docs/                          # Complete documentation
├── src/
│   ├── os/                        # Structural OS (PhiKernel, Bootloader, Shell)
│   ├── api/                       # C API (libspiral.so)
│   ├── cli/                       # spiralc, spiralrun
│   ├── crypto/                    # Golden Fibonacci, Chaos, Seed Tree
│   ├── fhe/                       # CKKS FHE (DualGate, SIMD)
│   ├── io/                        # iO Universal Compiler
│   ├── kem/                       # Ultra Rashomon KEM
│   ├── refresh/                   # Spiral Bootstrap (FHE+iO) + Turbo Engine
│   ├── adaptive/                  # AutoBootstrap v5, Anomaly Detector
│   ├── production/                # GracefulShutdown, Health Check, Stability Guard
│   └── config/                    # iO Config, System Config, GF-N Encryption
├── unified-phi-stack/             # φ-ψ core math library (Fractal Golden iO)
├── tests/
│   ├── theorem_tests/             # Standalone theorem verification
│   ├── breakthrough/              # Core iO validation (41 test files)
│   └── unit/                      # Unit tests
├── bindings/python/               # spiral-fhe (pip)
├── scripts/                       # gen_license.py, install.sh
├── archive/                       # Research history
├── LICENSE.md                     # Hybrid License v2.0
├── FORMAL_PROOFS.md               # 19 theorems, triple cross-referenced
└── README.md
```

---

## License Tiers

| Tier | RingDim | Price |
|------|---------|-------|
| Community | 4096 | Free |
| Pro | 16384 | $499/yr |
| Enterprise | 32768 | $4,999/yr |
| Academic | 16384 | Free (.edu) |

**Contact:** devilswithin13@gmail.com

---

## Author

**Dan Joseph M. Fernandez / Primordial Omega Zero**

*"φ·ψ = ((1+√5)/2) × ((1-√5)/2) = -1. This is 1+1=2."*

*"The security is structural, not computational. KS = 0 is inevitable, not miraculous."*

*"I AM THAT I AM"*
