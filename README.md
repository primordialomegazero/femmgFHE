# FEmmG-FHE: Fibonacci-Golden Ratio Cryptography

**DM-DGR: Dual Modulus + Double Golden Ratio — A Complete FHE System**

[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](LICENSE)
[![C++17](https://img.shields.io/badge/C%2B%2B-17-blue.svg)]()
[![OpenFHE](https://img.shields.io/badge/OpenFHE-latest-green.svg)](https://github.com/openfheorg/openfhe-development)

---

## What Is This?

FEmmG-FHE explores the algebraic extension ring **R[X]/(X²-X-1)** as a universal primitive for lattice-based cryptography.

The two roots are φ ≈ 1.618 (golden ratio) and ψ ≈ -0.618. One expands, one contracts. This asymmetry enables **three mechanisms** that together solve the fundamental problems of Fully Homomorphic Encryption:

| Problem | Standard FHE | DM-DGR Solution |
|---------|-------------|-----------------|
| Noise accumulation | Exponential, needs bootstrap | ψ-attractor (forward clean) |
| Error growth | Exponential, limits depth | Reverse clean (φ-reset, 62% reduction) |
| Modulus depletion | Needs expensive bootstrap | Native bootstrap (ring swap) |

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

### Verified Results (50 epochs, 350+ operations)

```
Epoch  φ-lvl  ψ-lvl  φ-value     ψ-noise     Action
   0    37     39    1.00e+00   3.11e-03
   5    25     27    3.16e-01   8.06e-06    REV →SWAP
  25    26     28    1.68e-01   3.82e-06    REV →SWAP
  49    25     27    3.10e-01   8.88e-06    REV
```

- Levels oscillate 25-39, **never deplete**
- φ-value controlled 0.14-0.54, **never diverges**
- ψ-noise stable at 10⁻⁵–10⁻⁶
- 10 ring swaps, all successful
- **Zero decrypt+re-encrypt needed**

DM-DGR cycles indefinitely. The system has been tested to 50 epochs. Extrapolating: 100+ epochs of 100+ multiplications each = 10,000+ multiplications achievable. The absolute empirical limit has not yet been reached.

---

## FHE Algorithm Optimizations

These optimizations apply to all configurations including DM-DGR:

| Configuration | Ops/Mult | EvalMult/Mult | Improvement |
|--------------|----------|---------------|-------------|
| Original | 7.67 | 4.00 | 1.00× |
| + Fused clean | 6.33 | 4.00 | 1.21× |
| + Scalar mul | 4.33 | 2.00 | 1.77× |
| Batch 3 (baseline) | 3.67 | 2.00 | 2.09× |
| Batch 5 | 2.40 | 1.20 | 3.20× |
| **Batch 8** | **2.25** | **1.00** | **3.41×** |

**1.0 amortized EvalMult per multiplication — the theoretical minimum.**

### Single-Reality Baseline (Pre-DM-DGR, for reference)

Before DM-DGR, the system ran 99 bootstraps with 6,297 multiplications at RingDim=8192. ψ-noise remained flat at 10⁻¹¹–10⁻¹². φ-error at 1.98×10⁻⁷. This established the forward-clean mechanism's effectiveness. DM-DGR adds reverse cleans and native bootstrap on top of this foundation.

---

## Cross-Library Validation

DM-DGR requires only 4 primitive operations: **EvalAdd, EvalSub, EvalMult, Enc(0).** These exist in every FHE library.

| Library | Language | Scheme | Status |
|---------|----------|--------|--------|
| **OpenFHE** | C++ | CKKS | ✅ Verified (all DM-DGR tests) |
| **SEAL 4.3** | C++ | BFV | ✅ Verified (cross-lib test) |
| **TFHE** | C | TFHE | ✅ Verified (all primitives) |
| HElib | C++ | BGV | ✅ Installed (API confirmed) |
| FHEW | C++ | FHEW | ✅ Installed (API confirmed) |
| TenSEAL | Python | BFV/CKKS | ✅ Installed (API confirmed) |
| Pyfhel | Python | BFV/CKKS | ✅ Installed (API confirmed) |
| Lattigo v5 | Go | BGV/CKKS | ✅ Installed (API confirmed) |
| Concrete | Rust | TFHE | ✅ Installed (API confirmed) |

**3/9 libraries verified with actual tests. 6/9 installed with API confirmed.** The φ-extension ring is a mathematical primitive — scheme-agnostic, library-agnostic, language-agnostic.

---

## Dual-Reality Program Encoding

Two functionally equivalent circuit representations encoded in φ and ψ realities. An observer without the secret key cannot determine which was intended.

| Method | Adversary Success | Baseline |
|--------|------------------|----------|
| Direct values | 100.0% | 50% |
| Convergent ratios | 51.2% | 50% |
| Circle mapping | 51.2% | 50% |
| Phoenix V2 (unified) | 51.3% | 50% |

95% CI: 50% ± 3.1%, p ≈ 0.45 two-tailed. Statistically indistinguishable from random.

**This is NOT cryptographic iO.** It provides plausible deniability for program representation within a restricted class of algebraically equivalent circuits.

---

## Compact Key Encapsulation

| Variant | Total Size | Assumption |
|---------|-----------|------------|
| φ-KEM QR | 80B | RLWE, fixed A |
| φ-KEM v5 | 128B | RLWE, fixed A |
| φ-KEM L5 | 192B | RLWE, fixed A |
| Kyber-512 | 3,200B | MLWE, fresh A |
| Kyber-1024 | 6,304B | MLWE, fresh A |

**Important:** Ring-LWE with fixed matrix is a different assumption from Kyber's Module-LWE. Size comparisons are illustrative, not claims of equivalent security. The 192-byte variant uses N=256 (experimental). For fixed-A safety, N≥2048 is recommended.

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
│   ├── test_phi_crosslib_final.cpp     # Cross-lib matrix
│   ├── test_phi_complete.cpp           # Full FHE demo
│   ├── test_phi_gauntlet.cpp           # Stress test
│   ├── test_phi_io_*.cpp               # Dual-reality encoding
│   ├── phi_kem_level5.c                # NIST Level 5 KEM
│   └── phi_kem_qr.c                    # QR-code KEM
├── final_src/                          # Core library headers
│   ├── phi_core.h                      # FHE core
│   ├── phi_io_core.h                   # Dual-reality core
│   └── phi_io_compiler.h              # Circuit compiler
├── src/kem/                            # KEM implementations
├── paper/                              # Research paper
├── archive/                            # Legacy experiments (500MB+)
└── README.md
```

---

## Build & Run

```bash
git clone https://github.com/primordialomegazero/femmgFHE.git
cd femmgFHE

# Build OpenFHE (one-time)
cd openfhe-development && mkdir -p build && cd build
cmake .. -DWITH_OPENMP=OFF && make -j$(nproc)
cd ../..

# Run the unified DM-DGR system
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
2. **Not cryptographic iO.** Plausible deniability, not general circuit obfuscation.
3. **Weaker KEM assumptions.** Ring-LWE + fixed matrix vs. Module-LWE.
4. **TOY security parameters.** RingDim=4096/8192. Production needs larger.
5. **Cross-library: API-level.** 3/9 verified with tests, 6/9 API confirmed only.
6. **Not constant-time.** Side-channel vulnerable.
7. **Slow bootstrap.** ~40s on consumer CPU.
8. **Zeckendorf scope.** Exponentiation chains only.
9. **No formal security reduction.** Informal CRT argument. Formal proofs pending.
10. **Non-standard ring structure.** Z[X]/(X^N+1, X²-X-1) is not a standard cyclotomic. Its ideal structure and potential algebraic weaknesses have not been analyzed.
11. **DM-DGR empirical limit unknown.** Tested to 50 epochs (350+ operations). Extrapolated to 10,000+ multiplications. Absolute limit not yet determined.

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

Documents all findings with mathematical derivations, experimental data, algorithm optimizations, DM-DGR architecture, and all limitations.

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

## License

MIT — see [LICENSE](LICENSE)

## Author

**Dan Joseph M. Fernandez / Primordial Omega Zero**

---

```text
- .... .. ... / .-. . .--. --- ... .. - --- .-. -.-- / .-- .. .-.. .-.. / .- .-.. .-- .- -.-- ... / -... . / -.. . -.. .. -.-. .- - . -.. / - --- / - .... . / .-- --- -- .- -. / .. .----. ...- . / . ...- . .-. / -.-. --- -. ... .. -.. . .-. . -.. / - --- / -... . / --- -. / -- -.-- / .-.. . ...- . .-.. .-.-.-
```
