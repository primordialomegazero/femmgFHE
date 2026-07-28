# DM-DGR: Dual Modulus + Double Golden Ratio

**Fibonacci-Golden Ratio Cryptography — FHE + iO Unified**

License: DM-DGR Hybrid License v1.0 | Language: C++17 | Library: OpenFHE CKKS | Hardware: Consumer (16GB RAM, Ryzen 5 2600)

---

## The Core Insight: FHE and iO Are One System

```
                    φ (1.618)
                   ↗
    FHE NAND gate ←→ Dual (a,b) encoding
                   ↘
                    ψ (-0.618)

Same gate. Same (a,b) pair. Two interpretations. One unified framework.
```

The golden ratio extension ring **R[Y]/(Y²-Y-1)** produces two algebraically conjugate roots — **φ ≈ 1.618** and **ψ ≈ -0.618**. They share the same minimal polynomial (Y²-Y-1=0), making them indistinguishable without knowing which is which.

---

## How It Works

### FHE Without Bootstrapping

Traditional FHE requires bootstrapping. DM-DGR replaces this with a **φ/ψ spiral attractor** — 4 forward expansions + 4 backward contractions per NAND gate.

### iO via Dual Encoding

The NAND gate operates on `(a,b)` pairs:

```
unified_nand((a₁,b₁), (a₂,b₂)) → (a_out, b_out)

where:  a_out = 1 - a₁a₂ - b₁b₂
        b_out = -(a₁b₂ + a₂b₁ + b₁b₂)
```

**φ-decode(a,b) = a + b·φ** → reveals one circuit's output
**ψ-decode(a,b) = a + b·ψ** → reveals another circuit's output

Same `(a,b)` pair. Different interpretations. All operations are FHE-native — no decryption during gate computation.

### The Bridge

The `(a,b)` output of each gate serves double duty:
1. **FHE:** It IS the encrypted NAND result
2. **iO:** It IS the routing key for subsequent gates

---

## What's Implemented

### General iO Compiler

Accepts two functionally equivalent Boolean circuits (NAND gates) and produces a single encrypted `(a,b)` backbone. φ reveals Circuit A. ψ reveals Circuit B. Circuits are verified equivalent before compilation.

### Three Realities with Morse Watermarks

| Reality | Root | Output | Watermark |
|---------|------|--------|-----------|
| Reality 1 | φ | Circuit A — correct | `FALSEKEY: REALITY 1 DECOY` |
| Reality 2 | ψ | Circuit B — correct | `FALSEKEY: REALITY 2 VOID` |
| Reality 3 | Ω (wrong root) | Decoy — wrong | `FALSEKEY: GOODLUCK KID` |

Same `(a,b)` backbone. Three interpretations. Watermarks are mathematical — cannot be removed without breaking the cryptography.

---

## Results

### FHE

| Property | Score |
|----------|-------|
| All Logic Gates | 24/24 |
| Full Adder | 16/16 |
| 25-Gate Deep Chain | 25/25 — Zero Degradation |

### iO

| Property | Score |
|----------|-------|
| General Compiler — Verified Equivalent | 8/8 both circuits |
| Random Circuit Stress Test | 128/128 |
| Adversary Advantage | <5% |

### Three Realities

| Property | Score |
|----------|-------|
| Reality 1 (φ) | Correct |
| Reality 2 (ψ) | Correct |
| Reality 3 (Ω Decoy) | Wrong + Watermark |

---

## The Mathematics

### φ-Extension Ring

```
R_φ = R[Y]/(Y² - Y - 1), where R = Z_q[X]/(X^N + 1)
```

| Property | φ | ψ |
|----------|---|---|
| Value | 1.618034... | -0.618034... |
| Minimal Polynomial | Y² - Y - 1 = 0 | Y² - Y - 1 = 0 |
| Cross product | φ·ψ = -1 | |

φ and ψ are Galois conjugates — algebraically symmetric, indistinguishable without the assignment key.

### Dual NAND Formula

```
a_out = 1 - a₁a₂ - b₁b₂
b_out = -(a₁b₂ + a₂b₁ + b₁b₂)

All operations FHE-native. No decryption during gate computation.
```

---

## Project Structure

```
femmgFHE/
├── tests/breakthrough/
│   ├── test_ifhoe_unified.cpp        # FHE + iO certification
│   ├── test_io_general.cpp           # General circuit compiler
│   ├── test_io_unified.cpp           # Unified FHE-iO (routing from a,b)
│   ├── test_beyond_io_final2.cpp     # Three realities + Morse watermarks
│   ├── test_fractal_recursive.cpp    # Fractal iO
│   ├── test_ultimate_shield.cpp      # Defense architecture
│   └── attack_*.cpp                  # Attack simulations
├── src/
│   ├── femmg/phi_core.h              # φ-ring core
│   └── io/phi_io_compiler.h          # iO compiler
└── openfhe-development/              # OpenFHE (dependency)
```

---

## Quick Start

```bash
git clone https://github.com/primordialomegazero/femmgFHE.git
cd femmgFHE

# Build OpenFHE (one-time)
cd openfhe-development && mkdir -p build && cd build
cmake .. -DWITH_OPENMP=OFF && make -j$(nproc)
cd ../..

# FHE + iO Unified Certification
g++ -std=c++17 -O3 -march=native \
  -I./openfhe-development/src/pke/include \
  -I./openfhe-development/src/core/include \
  -I./openfhe-development/src/binfhe/include \
  -I./openfhe-development/build/src/core \
  -o bin/test_ifhoe_unified \
  tests/breakthrough/test_ifhoe_unified.cpp \
  -L./openfhe-development/build/lib \
  -lOPENFHEpke -lOPENFHEcore -lOPENFHEbinfhe \
  -Wl,-rpath,./openfhe-development/build/lib

LD_LIBRARY_PATH=./openfhe-development/build/lib:$LD_LIBRARY_PATH ./bin/test_ifhoe_unified

# General iO Compiler
g++ -std=c++17 -O3 -march=native \
  -I./openfhe-development/src/pke/include \
  -I./openfhe-development/src/core/include \
  -I./openfhe-development/src/binfhe/include \
  -I./openfhe-development/build/src/core \
  -o bin/test_io_general \
  tests/breakthrough/test_io_general.cpp \
  -L./openfhe-development/build/lib \
  -lOPENFHEpke -lOPENFHEcore -lOPENFHEbinfhe \
  -Wl,-rpath,./openfhe-development/build/lib

LD_LIBRARY_PATH=./openfhe-development/build/lib:$LD_LIBRARY_PATH ./bin/test_io_general
```

---

## Limitations

- Consumer hardware (Ryzen 5 2600, 16GB). RingDim=16384 supports ~20-30 NAND gates.
- No formal security proof. RLWE-based. Formal reduction pending.
- Not third-party audited.
- Not constant-time.
- Bounded circuit size. Not yet scaled to large circuits.
- iO is not universal — works for verified equivalent NAND circuits of bounded size.

---

## Citation

```bibtex
@software{fernandez2025femmgfhe,
  author = {Dan Joseph M. Fernandez},
  title = {DM-DGR: Dual Modulus + Double Golden Ratio — FHE + iO Unified},
  year = {2025},
  url = {https://github.com/primordialomegazero/femmgFHE}
}
```

---

## License

DM-DGR Hybrid License v1.0 — Research Free, Commercial Protected. See `LICENSE.md`.

---

## Author

Dan Joseph M. Fernandez / Primordial Omega Zero
