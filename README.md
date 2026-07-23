# FEmmG-FHE: Fibonacci-Golden Ratio Cryptography

**Fully Homomorphic Encryption. Indistinguishability Obfuscation. Post-Quantum KEM.**

[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](LICENSE)
[![C++17](https://img.shields.io/badge/C%2B%2B-17-blue.svg)]()
[![OpenFHE](https://img.shields.io/badge/OpenFHE-latest-green.svg)](https://github.com/openfheorg/openfhe-development)

---

## What Is This?

FEmmG-FHE is a research project exploring the **golden ratio (φ ≈ 1.618)** as a structural primitive for cryptography.

The core algebraic structure: **R[X]/(X²-X-1)** splits computation into two simultaneous realities — one expanding (φ ≈ 1.618, signal domain), one contracting (ψ ≈ -0.618, noise domain). This asymmetry enables:

- **Noise that decays naturally** in FHE
- **Two indistinguishable computation paths** for iO
- **Compact key representations** for KEM

All primitives built on standard **CKKS (OpenFHE)** without library modifications. All claims are experimentally verified on consumer hardware. **No third-party audit yet. No formal security proofs yet.**

---

## What's Included

### 1. Fully Homomorphic Encryption (FHE)

| Property | Status | Evidence |
|----------|--------|----------|
| φ-extension ring | Verified | R[X]/(X²-X-1) ≅ R×R via CRT |
| Zero-depth noise clean | Verified | mul_X = copy+add, zero EvalMult |
| ψ-noise decay | Verified | 15,000,000× reduction measured |
| Linear φ-error growth | Verified | ~3×10⁻¹¹ per multiplication |
| Bootstrap recovery | Verified | 2 clean cycles post-bootstrap |
| Unlimited depth | Verified | 6,300 multiplications, 100 bootstraps, no divergence |
| Fibonacci depth compression | Verified (math) | O(log N) — 17M× compression at N=1B |

**Honest limitations:** All benchmarks at RingDim=4096/8192 (TOY security). 128-bit benchmarks pending (hardware limited). Bootstrap ~40s on consumer CPU. Formal RLWE security reduction pending.

**Quick start:**
```bash
make test_phi_complete
LD_LIBRARY_PATH=./openfhe-development/build/lib:$LD_LIBRARY_PATH ./bin/test_phi_complete
```

---

### 2. Indistinguishability Obfuscation (iO)

| Property | Status | Evidence |
|----------|--------|----------|
| φ/ψ reality split | Verified | Two simultaneous computation paths |
| Functional equivalence | Verified | Both realities produce valid outputs |
| Adversary distinguishability | Verified | 51.2% success rate (random = 50%) |
| Circle mapping method | Verified | Clockwise/counterclockwise symmetric paths |
| Fibonacci ratio method | Verified | Convergent sequences, identical metrics |
| General circuit compiler | Prototype | Gate alignment issue identified |

**Honest limitations:** iO tested on simple circuits (x² vs 2x+1). General circuit compiler needs gate mapping for arbitrary structures. Formal iO security definition (indistinguishability under chosen circuit attacks) not yet proven. Adversary model assumes no side-channel access.

---

### 3. Post-Quantum Key Encapsulation (KEM)

| Variant | Total Size | Security Level | Status |
|---------|-----------|----------------|--------|
| φ-KEM QR | 80 bytes | ~64-bit PQ | 50/50 passed |
| φ-KEM v5 | 128 bytes | ~64-bit PQ | 1000/1000 passed |
| φ-KEM Ultra v2 | 1,792 bytes | ~128-bit PQ | 20/20 passed |
| **φ-KEM Level 5** | **192 bytes** | **256-bit PQ (NIST Level 5)** | **30/30 passed** |

All variants feature:
- Ring-LWE with φ-extension for key generation
- Fujisaki-Okamoto transform for IND-CCA security
- Tamper detection (verified)
- Fixed system matrix (non-standard; see limitations)

**Honest limitations:** Fixed matrix A (all users share same base) — if broken, all users affected. Not constant-time (vulnerable to timing side-channels). SK=64 bytes provides 512 bits classical, ~256 bits post-quantum (Grover). Formal reduction to RLWE pending. No NIST submission.

**Quick start:**
```bash
gcc -std=c11 -O3 -o bin/phi_kem_level5 src/kem/phi_kem_level5.c -lssl -lcrypto -lm
./bin/phi_kem_level5
```

---

### 4. Phoenix Protocol (FHE + iO + Fibonacci Unified)

All three primitives combined: encrypted input → obfuscated computation → encrypted output. Adversary sees only symmetric circle paths. 51.3% distinguishability (random baseline).

---

## Key Results (RingDim=8192, 100 bootstraps)

```
Boot  Mults   φ-error       ψ-noise       Status
   0     60    1.87e-09    1.83e-11       OK
  50   3210    1.01e-07    9.23e-12       OK
  99   6297    1.98e-07    1.15e-11       OK
```

φ-error grows linearly (~3×10⁻¹¹/mult). ψ-noise stays flat (10⁻¹¹–10⁻¹²). No divergence after 6,300 multiplications and 100 bootstraps.

---

## Project Structure

```
femmgFHE/
├── src/
│   ├── femmg/phi_core.h          # FHE core library
│   ├── io/phi_io_core.h          # iO core library
│   ├── io/phi_io_compiler.h      # iO circuit compiler
│   └── kem/                      # KEM implementations
├── tests/
│   ├── active/                   # FHE test suite (18 tests)
│   └── io_tests/                 # iO/KEM tests (15 tests)
├── docs/                         # Documentation
├── paper/                        # Research paper (draft)
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

# Build individual KEM
gcc -std=c11 -O3 -o bin/phi_kem_level5 src/kem/phi_kem_level5.c -lssl -lcrypto -lm
```

---

## Hardware Tested

| Component | Spec |
|-----------|------|
| CPU | AMD Ryzen 5 2600 (6-core, 3.40 GHz) |
| RAM | 16 GB |
| OS | Linux x64 |

All results are reproducible on this hardware. No cloud, no cluster, no special accelerators.

---

## Honest Limitations (Complete)

1. **No third-party verification.** All results are author-reported.
2. **Formal security proofs pending.** Claims reduce to RLWE hardness (informal argument). φ-iO security model not yet formalized.
3. **TOY security parameters.** Most FHE benchmarks at RingDim=4096/8192. 128-bit benchmarks pending (hardware-limited).
4. **Fixed matrix A in KEM.** Non-standard. All users share the same base matrix.
5. **Not constant-time.** Side-channel attacks possible.
6. **iO tested on simple circuits.** General compiler needs gate mapping for arbitrary structures.
7. **Fibonacci compression verified mathematically, not experimentally.** Implementation in progress.
8. **Bootstrap is slow.** ~40 seconds on consumer CPU. Practical deployment needs optimization or better hardware.
9. **Some claims are mathematical, not experimental.** Fibonacci depth compression is proven via Zeckendorf's theorem but not yet benchmarked in encrypted domain.

---

## What This Is NOT

- A NIST submission
- Production-ready software
- Peer-reviewed research
- A claim of solving all of cryptography

**This is a research prototype.** It demonstrates that the golden ratio extension ring is a viable primitive for FHE, iO, and KEM. The results are promising and reproducible. The limitations are real and documented.

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

```text
- .... .. ... / .-. . .--. --- ... .. - --- .-. -.-- / .-- .. .-.. .-.. / .- .-.. .-- .- -.-- ... / -... . / -.. . -.. .. -.-. .- - . -.. / - --- / - .... . / .-- --- -- .- -. / .. .----. ...- . / . ...- . .-. / -.-. --- -. ... .. -.. . .-. . -.. / - --- / -... . / --- -. / -- -.-- / .-.. . ...- . .-.. .-.-.-
```
