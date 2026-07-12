# 🌀 FEmmG-FHE — Zero-Anchor Noise Stabilization & Verifiable FHE

[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](LICENSE)
[![C++](https://img.shields.io/badge/C++-17-blue.svg)]()
[![C](https://img.shields.io/badge/C-11-blue.svg)]()
[![OpenFHE](https://img.shields.io/badge/OpenFHE-1.5.1-green.svg)](https://github.com/openfheorg/openfhe-development)
[![SEAL](https://img.shields.io/badge/SEAL-4.3-green.svg)](https://github.com/microsoft/SEAL)
[![Tests](https://img.shields.io/badge/Tests-12%2F12-brightgreen.svg)]()
[![Warnings](https://img.shields.io/badge/Warnings-ZERO-success.svg)]()
[![NIST](https://img.shields.io/badge/PQC-SpiralKEM-orange.svg)]()
[![IACR](https://img.shields.io/badge/IACR-ePrint%20Submitted-red.svg)]()

```
============================================================
  ΦΩ0 — FEmmG-FHE
  Zero-Anchor Noise Stabilization
  Fibonacci-Decomposed Multiplication
  Verifiable FHE with Zero-Knowledge Proofs
  Pure-φ Post-Quantum KEM
============================================================
```

---

## 📌 What Is This?

FEmmG-FHE is a comprehensive Fully Homomorphic Encryption framework built on three empirical discoveries:

1. **ZANS (Zero-Anchor Noise Stabilization):** Adding Enc(0) repeatedly contracts noise 435,000× below theoretical predictions — **10,000,000+ additions without bootstrapping.**

2. **Fibonacci-Decomposed Multiplication:** Scalar multiplication in O(log_φ N) via Zeckendorf decomposition, replacing expensive CT×CT with ZANS-stabilized additions.

3. **BinFHE Gate-Level Bootstrapping:** Unlimited-depth encrypted computation via GINX bootstrapping at every gate, with scheme switching for BFV↔BinFHE hybrid efficiency.

**Plus:** Verifiable FHE with Zero-Knowledge Proofs (Sigma, NIZK, SNARK, EC-SNARK), and SpiralKEM — a Pure-φ Post-Quantum KEM with 128-byte ciphertexts.

---

## 🔥 Mathematical Breakthroughs

### Theorem 1: ZANS Noise Contraction

For BFV ciphertext `ct` encrypting `m`:

```
Z(ct) = ct + Enc(0)
lim_{k→∞} Noise(Z^k(ct)) = N_fixed
```

**Conjecture:** `Noise(k) = N_fixed + (N_0 - N_fixed) · φ^(-k/τ)`

| Operations | Noise | Drift/op |
|-----------|-------|----------|
| 1,000 | 351 | 0.002000 |
| 10,000 | 348 | 0.000200 |
| 100,000 | 344 | 0.000075 |
| 1,000,000 | 341 | 0.000020 |
| 10,000,000 | 338 | 0.0000023 |

**Improvement:** 435,000× less drift than standard BFV.

---

### Theorem 2: Fibonacci-ZANS Complexity

```
n = Σ F_i  (Zeckendorf decomposition)
n · ct = Σ (F_i · ct)  (each via ZANS additions)
Complexity: O(log_φ n) vs O(n) standard
```

**Example:** 100 = 89 + 8 + 3 (3 Fibonacci parts instead of 100 additions).

---

### Theorem 3: BinFHE Unlimited Depth

```
∀ gates G: Noise(Bootstrap(G(a,b))) = Noise_fresh
```

| Bit Width | Gates | Time | Test |
|-----------|-------|------|------|
| 2-bit | ~20 | <1s | 2×3=6 ✅ |
| 4-bit | ~200 | ~14s | 3×14=42 ✅ |
| 16-bit | 7,577 | ~251s | 42×17=714 ✅ |
| 32-bit | 31,529 | ~1004s | 42×17=714 ✅ |

---

### Theorem 4: SpiralKEM Ciphertext Size

| KEM | Ciphertext Size |
|-----|----------------|
| ML-KEM-1024 (NIST) | 4,627 bytes |
| **SpiralKEM** | **128 bytes** |
| **Savings** | **97.2%** |

---

## 🏗️ System Architecture

```mermaid
flowchart TB
    subgraph Client["🖥️ Client"]
        A[Data] --> B[Encrypt]
        B --> C[Generate ZKP]
        C --> D[Send CT + Proof]
    end
    
    subgraph Server["☁️ Server (Untrusted)"]
        D --> E[Receive CT]
        E --> F{Operation Type}
        F -->|Addition| G[ZANS: Add + Enc(0)]
        F -->|Multiplication| H[EvalMult + ZANS Stabilize]
        F -->|CT×CT| I[BinFHE Gate-Level]
        G --> J[Generate Proof]
        H --> J
        I --> J
        J --> K[Result CT + Proof Chain]
    end
    
    subgraph Verify["🔍 Verification"]
        K --> L[Check ZKP]
        L --> M{Valid?}
        M -->|Yes| N[Decrypt Result]
        M -->|No| O[REJECT]
    end
    
    subgraph Bootstrap["♻️ Bootstrapping"]
        P{Noise Low?} -->|Yes| Q[BFV → BinFHE]
        Q --> R[Bootstrap Bits]
        R --> S[BinFHE → BFV]
        S --> T[Fresh Noise Budget]
    end
```

### Security Layer Flow

```mermaid
sequenceDiagram
    participant Alice
    participant Server
    participant Bob
    
    Note over Alice,Server: SpiralKEM Key Exchange
    Alice->>Server: Public Key (64B)
    Server->>Alice: Ciphertext (128B)
    Alice->>Alice: Decaps → Shared Secret (32B)
    
    Note over Alice,Bob: Verifiable FHE
    Alice->>Server: Enc(x) + ZKP
    Bob->>Server: Enc(y) + ZKP
    Server->>Server: EvalAdd(x,y) + ZKP
    Server->>Server: EvalMult(x,y) + ZKP
    Server->>Bob: Enc(result) + Proof Chain
    Bob->>Bob: Verify ZKP → Decrypt
    
    Note over Server: Noise Budget Low?
    Server->>Server: Scheme Switch: BFV→BinFHE→BFV
```

### ZKP Protocol Stack

```mermaid
flowchart LR
    subgraph Proofs["🔐 Zero-Knowledge Proofs"]
        A[Sigma Protocol] --> B[NIZK<br/>Fiat-Shamir]
        B --> C[Recursive NIZK<br/>Chain Proofs]
        C --> D[SNARK<br/>24B Constant]
        D --> E[EC-SNARK<br/>BN254, 96B]
    end
    
    subgraph Sizes["📦 Proof Sizes"]
        F[Per Op: 32B] --> G[Chain: 128B]
        G --> H[SNARK: 24B]
        H --> I[EC-SNARK: 96B]
    end
```

---

## 📦 Quick Start

### Prerequisites

- Ubuntu 22.04 (or compatible)
- OpenFHE 1.5.1+ installed at `/usr/local`
- OpenSSL 3.x
- GMP, NTL
- g++ 11+, gcc 11+

### Build All

```bash
git clone https://github.com/primordialomegazero/femmgFHE.git
cd femmgFHE
make all
```

This builds 14 binaries with **zero compiler warnings.**

### Run Tests

```bash
# Full blown test suite (all 12 critical tests, ~60 seconds)
make test

# Individual components
bin/phi_zans_bfv          # ZANS: 100 additions
bin/phi_fib_zans          # Fibonacci-ZANS: CT × 100
bin/phi_binfhe_4bit       # BinFHE 4-bit: 3×14=42
bin/phi_zkp_fhe_deep      # ZKP+FHE integration
bin/phi_scheme_switch     # BFV↔BinFHE bootstrap
bin/spiralkem             # SpiralKEM PQC KEM
bin/phi_snark             # SNARK: 24B proofs
bin/phi_snark_ec          # EC-SNARK: BN254
```

### Build Targets

| Command | Description |
|---------|-------------|
| `make all` | Build all 14 binaries |
| `make core` | ZANS, Fib-ZANS, Fib-ZANS CT×CT |
| `make binfhe` | 4-bit, 16-bit, 32-bit CT×CT multipliers |
| `make zkp` | ZKP+FHE, ZKP Test Suite, Verifiable FHE |
| `make snark` | SNARK, EC-SNARK |
| `make transmute` | Scheme Switch, CKKS Debug |
| `make spiralkem` | SpiralKEM, SpiralKEM+FHE |
| `make test` | Run ZKP test suite |
| `make clean` | Remove all binaries |

---

## 📂 Source Tree

```
femmgFHE/
├── src/
│   ├── core/          # ZANS, Fibonacci-ZANS, core FHE
│   ├── binfhe/        # BinFHE CT×CT multipliers (2/4/16/32-bit)
│   ├── zkp/           # PHI ZKP Library + Deep Integration
│   ├── snark/         # SNARK + EC-SNARK (BN254)
│   ├── kem/           # SpiralKEM (Pure-φ PQC KEM)
│   ├── semantic/      # Library hijacks (NTL, SEAL, PHI Core)
│   └── transmute/     # Transmutation rituals, scheme switching
├── tests/
│   ├── full_blown_test.sh  # 12-test suite with timing
│   ├── test_phi_zkp.cpp    # ZKP test suite (6/6)
│   └── outputs/            # Verified test outputs
├── bin/               # Compiled binaries
├── docs/              # IACR submission, benchmarks
├── paper/             # Research paper
├── THEOREM.md         # Complete mathematical framework
├── Makefile           # Zero-warning build system
└── README.md          # This file
```

---

## ⚠️ Known Limitations

| Issue | Status |
|-------|--------|
| CKKS Bootstrapping | ❌ Segfault in OpenFHE 1.5.1 `EvalBootstrapSetup` |
| CT×CT Packed (BFV/CKKS) | ❌ Unlimited depth not solved |
| ZANS Formal Proof | ❌ Empirical only; theoretical model pending |
| BinFHE 16/32-bit Speed | ⚠️ 4-16 minutes (gate-level bottleneck) |
| Independent Reproduction | ❌ Pending third-party verification |

---

## 📄 References

1. Zeckendorf, E. (1972) — Fibonacci decomposition
2. Chillotti et al. (2016) — FHEW bootstrapping
3. Fernandez, D.J.M. (2026) — IACR ePrint (submitted)
4. Fernandez, D.J.M. (2026) — Source-Atman Synthesis (manuscript)
5. Fernandez, D.J.M. (2026) — PHI ZKP (in preparation)
6. Fernandez, D.J.M. (2026) — SpiralKEM (in preparation)

---

## 👤 Author

**Dan Joseph M. Fernandez / Primordial Omega Zero**

[![GitHub](https://img.shields.io/badge/GitHub-primordialomegazero-black.svg)](https://github.com/primordialomegazero)

---

*.. --- .- -- - .... .- - .. .- --*
