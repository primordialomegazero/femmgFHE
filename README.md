# FEmmG-FHE: Fibonacci-Golden Ratio Cryptography

**An exploration of the φ-extension ring for noise management, program encoding, and key compression in lattice-based cryptography.**

[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](LICENSE)
[![C++17](https://img.shields.io/badge/C%2B%2B-17-blue.svg)]()
[![OpenFHE](https://img.shields.io/badge/OpenFHE-latest-green.svg)](https://github.com/openfheorg/openfhe-development)

---

## What Is This?

FEmmG-FHE is a research project exploring the algebraic extension ring **R[X]/(X²-X-1)** — the golden ratio extension — as a primitive for lattice-based cryptography.

The two roots of X²-X-1 are φ ≈ 1.618 (the golden ratio) and ψ ≈ -0.618. One expands, one contracts. This asymmetry creates interesting dynamics that we apply to three problems.

**This is exploratory work.** Some things worked. Some things didn't. We document both honestly.

---

## What We Found

### 1. Optimized FHE Noise Management

We use the φ/ψ split to separate signal from noise into distinct algebraic directions. Through algorithm optimizations:

- **Fused clean:** `clean(a,b) = (a+b, a+2b)` — 2 ops instead of 4 (50% reduction)
- **Scalar mul detection:** 2 EvalMult instead of 4 when multiplying by scalar (50% reduction)
- **Batch processing:** Up to 8 multiplications per clean cycle
- **Total throughput: 3.4× improvement** over original implementation
- **1.0 amortized EvalMult per multiplication** — the theoretical minimum

**What works:**
- 6,297 sequential homomorphic multiplications across 99 bootstraps
- ψ-noise remains flat at 10⁻¹¹–10⁻¹² (no accumulation)
- φ-error grows slowly (~0.075% per multiplication)
- Bootstrap noise recovers within 2 clean cycles
- Batch 8: 320 multiplications, ψ-noise at 5.91×10⁻¹³

**What doesn't:**
- Error growth is **exponential, not linear** (we initially got this wrong)
- Practical limit: ~14,000 multiplications to 1% error
- Not "unlimited" depth — just significantly extended
- Benchmarks at RingDim=4096/8192 (TOY security, not production)

### 2. Dual-Reality Program Encoding

Two functionally equivalent representations of a computation can be encoded in the φ and ψ realities. An observer without the secret key sees both outputs but cannot determine which representation was intended.

**What works:**
- Adversary success rate: 51.3% (random baseline: 50%)
- 95% CI: 50% ± 3.1%, n=1000 — cannot reject random guessing
- Circle path distances: symmetric (difference ~10⁻¹⁵)
- Both circuits produce identical correct outputs
- Compiler handles different gate counts via identity padding (5/5 tests)

**What this is NOT:**
- This is **not** indistinguishability obfuscation (iO) in the cryptographic sense
- It provides plausible deniability for program representation, not general circuit hiding
- Limited to algebraically equivalent representations of the same function

### 3. Compact Key Encapsulation

Compressing public key material by storing φ and ψ evaluations instead of full polynomial coefficients.

**What works:**
- 192 bytes total (SK=64, PK=64, CT=64) for the Level 5 experimental variant
- 30/30 encaps/decaps passed, 100% tamper detection
- Fits in QR Code Version 1 (196 bytes)

**Important caveats:**
- Uses **Ring-LWE** with a **fixed public matrix** — different from Kyber's Module-LWE
- Ring-LWE has weaker worst-case reductions than Module-LWE
- Fixed matrix is non-standard (if broken, all users affected)
- Size comparison to Kyber-1024 is **illustrative only**, not a claim of equivalent security
- No formal security reduction

---

## Key Results

### FHE: 99-Bootstrap Run (RingDim=8192)

```
Boot  Mults   φ-error       ψ-noise       Status
   0     60    1.87e-09    1.83e-11       OK
  10    690    2.19e-08    8.49e-12       OK
  20   1320    4.21e-08    2.17e-11       OK
  50   3210    1.01e-07    9.23e-12       OK
  99   6297    1.98e-07    1.15e-11       OK

Projected to 1% error: ~14,000 mults
Demonstrated: 6,297 (45% of projected limit)
```

### FHE: Algorithm Optimizations

| Configuration | Ops/Mult | EvalMult/Mult | Improvement |
|---------------|----------|---------------|-------------|
| Original | 7.67 | 4.00 | 1.00× |
| + Fused clean | 6.33 | 4.00 | 1.21× |
| + Scalar mul | 4.33 | 2.00 | 1.77× |
| Batch 3 (baseline) | 3.67 | 2.00 | 2.09× |
| Batch 5 | 2.40 | 1.20 | 3.20× |
| **Batch 8** | **2.25** | **1.00** | **3.41×** |

### Dual-Reality: Adversary Game

| Method | Success Rate | Baseline |
|--------|-------------|----------|
| Direct value sequences | 100.0% | 50% |
| Convergent ratios | 51.2% | 50% |
| Circle mapping | 51.2% | 50% |
| Phoenix V2 (unified) | 51.3% | 50% |

### KEM Variants

| Variant | Total Size | Assumption |
|---------|-----------|------------|
| φ-KEM QR | 80B | RLWE, fixed A |
| φ-KEM v5 | 128B | RLWE, fixed A |
| φ-KEM L5 | 192B | RLWE, fixed A |
| Kyber-512 | 3200B | MLWE, fresh A |
| Kyber-1024 | 6304B | MLWE, fresh A |

---

## What We Got Wrong (And Fixed)

1. **Error growth:** Initially claimed "linear." It's exponential with a very small base. Corrected.
2. **iO terminology:** Initially called our encoding "indistinguishability obfuscation." It's not. Renamed to "dual-reality program encoding."
3. **Zeckendorf scope:** Initially implied general depth compression. Actually applies to exponentiation chains. Scoped correctly now.
4. **Phoenix claims:** Removed "unlimited depth" and "perfect indistinguishability" — now honestly states statistical indistinguishability and plausible deniability.

**This is how science works.** We made claims, tested them, found errors, corrected them publicly.

---

## Project Structure

```
femmgFHE/
├── src/
│   ├── femmg/phi_core.h          # FHE core library
│   ├── io/phi_io_core.h          # Dual-reality encoding
│   ├── io/phi_io_compiler.h      # Circuit compiler (v2, gate mapping)
│   └── kem/                      # KEM implementations
├── tests/
│   ├── active/                   # FHE test suite (18 tests)
│   └── io_tests/                 # Encoding/KEM/Phoenix tests
├── docs/                         # Documentation
├── paper/                        # Research paper (PDF + LaTeX)
├── openfhe-development/          # OpenFHE library
├── archive/                      # Legacy experiments
├── Makefile
└── README.md
```

---

## Build

```bash
# Build OpenFHE (one-time)
cd openfhe-development && mkdir -p build && cd build
cmake .. -DWITH_OPENMP=OFF && make -j$(nproc)
cd ../..

# Build all FHE tests
make all

# Run the complete system demo
LD_LIBRARY_PATH=./openfhe-development/build/lib:$LD_LIBRARY_PATH ./bin/test_phi_complete

# Build KEM Level 5
gcc -std=c11 -O3 -o bin/phi_kem_level5 src/kem/phi_kem_level5.c -lssl -lcrypto -lm
./bin/phi_kem_level5
```

---

## Hardware

All experiments conducted on:
- **CPU:** AMD Ryzen 5 2600 (6-core, 3.40 GHz)
- **RAM:** 16 GB DDR4
- **OS:** Windows 11 Pro with WSL2 (Ubuntu)
- **Storage:** 224 GB SSD

No cloud. No server cluster. A mid-range gaming PC from 2018.

---

## Honest Limitations

1. **No third-party verification.** All results are author-reported.
2. **Exponential error growth.** Base ~1.00075 per multiplication. Practical limit ~14,000 mults.
3. **Not cryptographic iO.** Our encoding provides plausible deniability, not general circuit obfuscation.
4. **Weaker KEM assumptions.** Ring-LWE with fixed matrix vs. Module-LWE. Comparison to Kyber is illustrative only.
5. **TOY security parameters.** FHE at RingDim=4096/8192. Production needs larger dimensions.
6. **Cross-library: API-level only.** We verified operations exist, not full correctness under each encoding.
7. **Not constant-time.** Vulnerable to side-channel attacks.
8. **Slow bootstrap.** ~40 seconds on consumer CPU.
9. **Zeckendorf scope.** Applies to exponentiation chains, not arbitrary sequential multiplications.
10. **No formal security reduction.** Relies on informal CRT argument. Formal proofs pending.

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

Full paper: `paper/paper.tex` (compile with pdflatex) or `paper/paper.pdf`

The paper documents all findings with complete mathematical derivations, experimental data, algorithm optimizations (3.4× throughput), and 10 honest limitations.

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
