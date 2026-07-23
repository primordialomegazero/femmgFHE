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

### 1. FHE Noise Management

We use the φ/ψ split to separate signal from noise into distinct algebraic directions. A "clean" operation using only addition (zero ciphertext multiplication depth) attenuates the ψ-component by ~0.382× per cycle.

**What works:**
- 6,297 sequential homomorphic multiplications across 99 bootstraps
- ψ-noise remains flat at 10⁻¹¹–10⁻¹² (no accumulation)
- φ-error grows slowly (~0.075% per multiplication)
- Bootstrap noise recovers within 2 clean cycles

**What doesn't:**
- Error growth is **exponential, not linear** (we initially got this wrong)
- Practical limit: ~14,000 multiplications to 1% error
- Not "unlimited" depth — just significantly extended
- Benchmarks at RingDim=4096/8192 (TOY security, not production)

### 2. Dual-Reality Program Encoding

Two functionally equivalent representations of a computation can be encoded in the φ and ψ realities. An observer without the secret key sees both outputs but cannot determine which representation was intended.

**What works:**
- Adversary success rate: 51.2% (random baseline: 50%)
- 95% CI: 50% ± 3.1%, n=1000, p=0.45 — cannot reject random guessing
- Fibonacci circle mapping hides direction (normal vs. reverse)

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

## Key Results (RingDim=8192, 99 Bootstraps)

```
Boot  Mults   φ-error       ψ-noise       Status
   0     60    1.87e-09    1.83e-11       OK
  10    690    2.19e-08    8.49e-12       OK
  20   1320    4.21e-08    2.17e-11       OK
  30   1950    6.16e-08    6.93e-12       OK
  40   2580    8.13e-08    9.11e-12       OK
  50   3210    1.01e-07    9.23e-12       OK
  60   3840    1.21e-07    8.93e-12       OK
  70   4470    1.41e-07    5.50e-12       OK
  80   5100    1.60e-07    2.13e-12       OK
  90   5730    1.80e-07    4.51e-12       OK
  99   6297    1.98e-07    1.15e-11       OK
```

φ-error grows exponentially with base ~1.00075 (0.075% per multiplication). ψ-noise flat. No divergence.

---

## What We Got Wrong (And Fixed)

1. **Error growth:** Initially claimed "linear." It's exponential with a very small base. Corrected in the current version.
2. **iO terminology:** Initially called our encoding "indistinguishability obfuscation." It's not. Renamed to "dual-reality program encoding."
3. **Zeckendorf scope:** Initially implied general depth compression. Actually applies to exponentiation chains. Scoped correctly now.

**This is how science works.** We made claims, tested them, found errors, corrected them publicly.

---

## Project Structure

```
femmgFHE/
├── src/
│   ├── femmg/phi_core.h          # FHE core library
│   ├── io/phi_io_core.h          # Dual-reality encoding
│   ├── io/phi_io_compiler.h      # Circuit compiler (prototype)
│   └── kem/                      # KEM implementations
├── tests/
│   ├── active/                   # FHE test suite (18 tests)
│   └── io_tests/                 # Encoding/KEM tests (15 tests)
├── docs/                         # Documentation
├── paper/                        # Research paper (PDF)
├── openfhe-development/          # OpenFHE library
├── archive/                      # Legacy experiments (523MB)
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
- **OS:** Windows 11 Pro (25H2) with WSL2 (Ubuntu)
- **Storage:** 224 GB SSD

No cloud. No server cluster. A mid-range gaming PC from 2018.

---

## Honest Limitations

1. **No third-party verification.** All results are author-reported.
2. **Exponential error growth.** Not linear as we initially claimed.
3. **Not iO.** Our encoding provides plausible deniability, not general circuit obfuscation.
4. **Weaker KEM assumptions.** Ring-LWE with fixed matrix A, not Module-LWE. Comparison to Kyber is illustrative only.
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

Full paper: `paper/paper.tex` (compile with pdflatex)

The paper documents all findings with complete mathematical derivations, experimental data, and honest limitations. We corrected significant errors between versions (see Section 1: "What We Did NOT Find").

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
