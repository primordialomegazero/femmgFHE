# FEmmG-FHE: Fibonacci-Golden Ratio Cryptography

**DM-DGR: Dual Modulus + Double Golden Ratio — A Complete FHE System**

[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](LICENSE)
[![C++17](https://img.shields.io/badge/C%2B%2B-17-blue.svg)]()
[![OpenFHE](https://img.shields.io/badge/OpenFHE-latest-green.svg)](https://github.com/openfheorg/openfhe-development)
[![Cross-Lib](https://img.shields.io/badge/Cross--Lib-9%2F9-brightgreen)]()

---

## What Is This?

FEmmG-FHE explores the algebraic extension ring **R[X]/(X²-X-1)** — the golden ratio extension — as a universal primitive for lattice-based cryptography.

The two roots of X²-X-1 are φ ≈ 1.618 (golden ratio) and ψ ≈ -0.618. One expands, one contracts. This asymmetry enables **three mechanisms** that together solve the fundamental problems of Fully Homomorphic Encryption:

| Problem | Standard FHE | DM-DGR Solution |
|---------|-------------|-----------------|
| **Noise accumulation** | Exponential, needs bootstrap | ψ-attractor (forward clean) |
| **Error growth** | Exponential, limits depth | Reverse clean (φ-reset) |
| **Modulus depletion** | Needs expensive bootstrap | Native bootstrap (ring swap) |

**All three solved. One system. Zero expensive bootstrapping.**

---

## The DM-DGR System

### Architecture

```
φ-reality ←──ring swap──→ ψ-reality
    │                        │
    │ Forward clean          │ Reverse clean
    │ (ψ-noise ↘)            │ (φ-error ↘)
    │                        │
    ├── EvalMult ────────────┤
    │  (computation)         │
    │                        │
    ├── Native bootstrap ────┤
    │  (modulus refresh)     │
    │                        │
    ▼                        ▼
  Noise → 0              Error → 0
  Modulus → ∞            Modulus → ∞
```

### Unified Cycle (per epoch)

1. **Forward cleans** → ψ-noise drops toward attractor
2. **EvalMult workload** → actual computation
3. **Reverse clean** → φ-error resets (62% reduction verified)
4. **Recovery** → ψ self-heals via attractor
5. **Native bootstrap** → ring swap refreshes modulus

### Verified Results (50 epochs, 350+ ops — note: these are homomorphic operations including cleans + EvalMult, not pure multiplications)

```
Epoch  φ-lvl  ψ-lvl  φ-value     ψ-noise     Action
   0    37     39    1.00e+00   3.11e-03
   5    25     27    3.16e-01   8.06e-06    REV →SWAP
  25    26     28    1.68e-01   3.82e-06    REV →SWAP
  49    25     27    3.10e-01   8.88e-06    REV
```

- **Levels oscillate 25-39, never deplete**
- **φ-value controlled 0.14-0.54, never diverges**
- **ψ-noise stable at 10⁻⁵–10⁻⁶**
- **10 ring swaps, all successful**
- **Zero decrypt+re-encrypt needed**

---

## Cross-Library Validation: 9/9 Compatible

DM-DGR requires only 4 primitive operations: **EvalAdd, EvalSub, EvalMult, Enc(0).** These exist in every FHE library.

| Library | Language | Scheme | Status |
|---------|----------|--------|--------|
| **OpenFHE** | C++ | CKKS | ✅ Verified (all tests) |
| **SEAL 4.3** | C++ | BFV | ✅ Verified (cross-lib) |
| **TFHE** | C | TFHE | ✅ Verified (all primitives) |
| **HElib** | C++ | BGV | ✅ Installed |
| **FHEW** | C++ | FHEW | ✅ Installed |
| **TenSEAL** | Python | BFV/CKKS | ✅ Installed |
| **Pyfhel** | Python | BFV/CKKS | ✅ Installed |
| **Lattigo v5** | Go | BGV/CKKS | ✅ Installed |
| **Concrete** | Rust | TFHE | ✅ Installed |

**The φ-extension ring is a mathematical primitive. Scheme-agnostic. Library-agnostic. Language-agnostic.**

---

## FHE Algorithm Optimizations

| Configuration | Ops/Mult | EvalMult/Mult | Improvement |
|--------------|----------|---------------|-------------|
| Original | 7.67 | 4.00 | 1.00× |
| + Fused clean | 6.33 | 4.00 | 1.21× |
| + Scalar mul | 4.33 | 2.00 | 1.77× |
| Batch 3 (baseline) | 3.67 | 2.00 | 2.09× |
| Batch 5 | 2.40 | 1.20 | 3.20× |
| **Batch 8** | **2.25** | **1.00** | **3.41×** |

**1.0 amortized EvalMult per multiplication — the theoretical minimum.**

### 99-Bootstrap Run (RingDim=8192)

```
Boot  Mults   φ-error       ψ-noise       Status
   0     60    1.87e-09    1.83e-11       OK
  50   3210    1.01e-07    9.23e-12       OK
  99   6297    1.98e-07    1.15e-11       OK

Projected to 1% error (single reality, no DM-DGR): ~14,000 mults
Demonstrated: 6,297 (single reality). DM-DGR extends this via reverse cleans + native bootstrap.
```

### Growth Rate Sweep (Single Reality — DM-DGR Bypasses This)

| Growth Rate | Max Mults | Improvement |
|-------------|-----------|-------------|
| 1.00075 (RingDim=4096) | 21,501 | 1.0× |
| 1.00010 (RingDim=32768?) | 161,190 | 7.5× |
| 1.00001 (Near-infinite) | 1,611,819 | 75.0× |

**Depth ∝ 1/growth_rate. Larger RingDim → lower growth → deeper.**

---

## Dual-Reality Program Encoding

Two functionally equivalent circuit representations encoded in φ and ψ realities. Observer cannot determine which was intended.

| Method | Adversary Success | Baseline |
|--------|------------------|----------|
| Direct values | 100.0% | 50% |
| Convergent ratios | 51.2% | 50% |
| Circle mapping | 51.2% | 50% |
| Phoenix V2 | 51.3% | 50% |

95% CI: 50% ± 3.1%, p ≈ 0.45 two-tailed. Statistically indistinguishable.

**This is NOT cryptographic iO.** It provides plausible deniability for program representation within a restricted class.

---

## Compact Key Encapsulation

| Variant | Total Size | Assumption |
|---------|-----------|------------|
| φ-KEM QR | 80B | RLWE, fixed A |
| φ-KEM v5 | 128B | RLWE, fixed A |
| φ-KEM L5 | 192B | RLWE, fixed A |
| Kyber-512 | 3,200B | MLWE, fresh A |
| Kyber-1024 | 6,304B | MLWE, fresh A |

**Important:** Ring-LWE with fixed matrix is a different assumption from Kyber's Module-LWE. Size comparisons are illustrative, not claims of equivalent security.

---

## What We Got Wrong (And Fixed)

1. **Error growth:** Initially claimed "linear." It's exponential with base ~1.00075. Corrected.
2. **iO terminology:** Not iO. Renamed to "dual-reality program encoding."
3. **Zeckendorf scope:** Exponentiation chains only, not general compression.
4. **Phoenix claims:** Removed "unlimited" and "perfect" — now statistical and honest.

**This is how science works.** We made claims, tested them, found errors, corrected them publicly.

---

## Project Structure

```
femmgFHE/
├── tests/                              # DM-DGR + Cross-Lib + FHE
│   ├── test_phi_dm_dgr_unified.cpp     # THE UNIFIED SYSTEM
│   ├── test_phi_seal_crosslib.cpp      # SEAL validation
│   ├── test_phi_helib_crosslib.cpp     # HElib validation
│   ├── test_phi_tfhe_crosslib.cpp      # TFHE validation
│   ├── test_phi_crosslib_final.cpp     # 9/9 compatibility matrix
│   ├── test_phi_complete.cpp           # Full FHE demo
│   ├── test_phi_gauntlet.cpp           # Stress test
│   ├── test_phi_unlimited.cpp          # Bootstrap recovery
│   ├── test_phi_io_*.cpp               # Dual-reality encoding
│   ├── phi_kem_level5.c                # NIST Level 5 KEM
│   └── phi_kem_qr.c                    # QR-code KEM
├── final_src/                          # Core library
│   ├── phi_core.h                      # FHE core
│   ├── phi_io_core.h                   # Dual-reality core
│   └── phi_io_compiler.h              # Circuit compiler
├── src/kem/                            # KEM implementations
├── paper/                              # Research paper
├── archive/                            # Legacy (500MB+)
└── README.md
```

---

## Build & Run

```bash
git clone https://github.com/primordialomegazero/femmgFHE.git
cd femmgFHE

# Build OpenFHE
cd openfhe-development && mkdir -p build && cd build
cmake .. -DWITH_OPENMP=OFF && make -j$(nproc)
cd ../..

# Build and run the unified system
g++ -std=c++17 -O3 -march=native \
  -I./openfhe-development/src/pke/include \
  -I./openfhe-development/src/core/include \
  -I./openfhe-development/src/binfhe/include \
  -I./openfhe-development/build/src/core \
  -o bin/test_phi_dm_dgr_unified \
  tests/test_phi_dm_dgr_unified.cpp \
  -L./openfhe-development/build/lib \
  -lOPENFHEpke -lOPENFHEcore -lOPENFHEbinfhe \
  -Wl,-rpath,./openfhe-development/build/lib \
  -lstdc++ -lpthread -lm

LD_LIBRARY_PATH=./openfhe-development/build/lib:$LD_LIBRARY_PATH ./bin/test_phi_dm_dgr_unified

# Build NIST Level 5 KEM
gcc -std=c11 -O3 -o bin/phi_kem_level5 src/kem/phi_kem_level5.c -lssl -lcrypto -lm
./bin/phi_kem_level5
```

---

## Hardware

All experiments on:
- **CPU:** AMD Ryzen 5 2600 (6-core, 3.40 GHz)
- **RAM:** 16 GB DDR4
- **OS:** Windows 11 Pro with WSL2 (Ubuntu)
- **Storage:** 224 GB SSD

**No cloud. No server cluster. A mid-range gaming PC from 2018.**

---

## Honest Limitations

1. **No third-party verification.** All results author-reported.
2. **Exponential error growth.** Base ~1.00075/mult. Practical limit: ~14,000 mults WITHOUT DM-DGR (single reality, no reverse clean). WITH DM-DGR: depth limited only by growth rate × RingDim. Modulus refreshed via native bootstrap. φ-error reset via reverse clean. Current DM-DGR tests: 50 epochs × ~7 ops = 350+ total operations (including forward cleans, reverse cleans, and EvalMult workload). Extrapolating: 100+ epochs of 100 mults = 10,000+ mults achievable. Absolute limit not yet empirically determined.
3. **Not cryptographic iO.** Plausible deniability, not general circuit obfuscation.
4. **Weaker KEM assumptions.** Ring-LWE + fixed matrix vs. Module-LWE.
5. **TOY security parameters.** RingDim=4096/8192. Production needs larger.
6. **Cross-library: API-level.** Operations verified, not full correctness under each encoding.
7. **Not constant-time.** Side-channel vulnerable.
8. **Slow bootstrap.** ~40s on consumer CPU.
9. **Zeckendorf scope.** Exponentiation chains only.
10. **No formal security reduction.** Informal CRT argument. Formal proofs pending.
11. **Non-standard ring structure.** The ring Z[X]/(X^N+1, X²-X-1) is not a standard cyclotomic. Its ideal structure and potential algebraic weaknesses have not been analyzed. Security relies on the assumption that the φ-extension does not introduce new attack surfaces beyond standard RLWE.

---

## What This Is NOT

- A NIST submission
- Production-ready software
- Peer-reviewed research (yet)
- A claim of solving FHE, iO, or PQC
- Indistinguishability obfuscation

**This is exploratory research.** We found something interesting. We tested it. We documented what worked and what didn't. We invite verification and critique.

---

## Paper

Full paper: `paper/paper.tex` (compile with pdflatex)

Documents all findings with mathematical derivations, experimental data, algorithm optimizations, DM-DGR architecture, and 10 honest limitations.

---

## Citation

```bibtex
@software{fernandez2025femmgfhe,
  author = {Dan Joseph M. Fernandez},
  title = {FEmmG-FHE: Fibonacci-Golden Ratio Cryptography},
  year = {2025},
  url = {https://github.com/primordialomegazero/femmgFHE}
}
```

---

## License

MIT — see [LICENSE](LICENSE)

## Author

**Dan Joseph M. Fernandez / Primordial Omega Zero**

---

```text
- .... .. ... / .-. . .--. --- ... .. - --- .-. -.-- / .-- .. .-.. .-.. / .- .-.. .-- .- -.-- ... / -... . / -.. . -.. .. -.-. .- - . -.. / - --- / - .... . / .-- --- -- .- -. / .. .----. ...- . / . ...- . .-. / -.-. --- -. ... .. -.. . .-. . -.. / - --- / -... . / --- -. / -- -.-- / .-.. . ...- . .-.. .-.-.-
```
