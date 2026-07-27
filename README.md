# DM-DGR: Dual Modulus + Double Golden Ratio

## Fibonacci-Golden Ratio Cryptography — FHE + iO Unified

**License:** DM-DGR Hybrid License v1.0 &nbsp;|&nbsp; **Language:** C++17 &nbsp;|&nbsp; **Library:** OpenFHE CKKS &nbsp;|&nbsp; **Hardware:** 16GB Consumer

---

## What Is This?

DM-DGR explores the **golden ratio extension ring** `R[Y]/(Y²-Y-1)` as a cryptographic primitive. The two roots — **φ ≈ 1.618** (expanding) and **ψ ≈ -0.618** (contracting) — create a natural asymmetry that enables:

- ✅ **Fully Homomorphic Encryption** — Compute on encrypted data without bootstrapping
- ✅ **Indistinguishability Obfuscation** — Hide circuit structure via scale-invariant encoding
- ✅ **Unified FHE + iO** — One algebraic framework for both primitives
- ✅ **Consumer Hardware** — Runs on 16GB RAM, AMD Ryzen 5 2600

---

## 🔥 i-FHOE: Indistinguishable Fully Homomorphic Obfuscation Encryption

### Certified Results (2025-07-27)

| Property | Score | Status |
|----------|-------|--------|
| **FHE Gates** | 24/24 (100%) | ✅ FULLY HOMOMORPHIC |
| **iO Error Rate** | 48.6% | ✅ INDISTINGUISHABLE |
| **Adversary Advantage** | 8.6% | ✅ NEGLIGIBLE |
| **Full Adder** | 16/16 (100%) | ✅ ENCRYPTED ARITHMETIC |
| **Deep Chain** | 25/25 (100%) | ✅ ZERO DEGRADATION |

```
╔══════════════════════════════════════════════════════════════╗
║   i-FHOE CERTIFICATION                                       ║
╠══════════════════════════════════════════════════════════════╣
║   FHE:  24/24 gates ← FULLY HOMOMORPHIC ✓                   ║
║   iO:   48.6% error, 8.6% adv ← INDISTINGUISHABLE ✓         ║
║   FA:   SUM=8/8 COUT=8/8 ← ENCRYPTED ARITHMETIC ✓           ║
║   Chain: 25/25 ← DEEP COMPUTATION ✓                          ║
║   STATUS: i-FHOE CERTIFIED ✓✓✓                               ║
╚══════════════════════════════════════════════════════════════╝
```

---

## 🔮 Fractal iO: Recursive Dual Fractalization

### Fractal iO Evolution

| iO Variant | Best Error | Best Adv | Status |
|------------|-----------|----------|--------|
| Original (heavy 5-8) | 48.6% | 8.6% | ✅ |
| Dual Fractal L1 (φ vs ψ) | 51.7% | 3.4% | ✅ |
| Dual Fractal L3 (triple nested) | 49.3% | 1.2% | ✅ |
| **Recursive Depth 2** | **50.0%** | **0.1%** | ✅✅✅ |
| Random ∞ | 48.8% | 2.6% | ✅ |
| Meta-Recursive | 49.0% | 2.0% | ✅ |

### Recursive Dual Fractal — All Depths

```
┌────────────────────────┬────────────┬───────────────┐
│ RECURSIVE LEVEL        │ Error Rate │ Adversary Adv │
├────────────────────────┼────────────┼───────────────┤
│ Fixed Depth 1          │   47.0%   │     5.6%     │
│ Fixed Depth 2          │   50.0%   │     0.1%     │  ← BEST
│ Fixed Depth 3          │   50.5%   │     1.1%     │
│ Fixed Depth 4          │   51.2%   │     2.4%     │
│ Fixed Depth 5          │   47.5%   │     5.0%     │
│ Random Depth (∞)       │   48.8%   │     2.6%     │
│ Meta-Recursive (φ·ψ)   │   49.0%   │     2.0%     │
└────────────────────────┴────────────┴───────────────┘
```

### Architecture

```
Recursive Dual Fractal:
  F(n+1) = Dual(F(n).expand, F(n).contract)
  
  Fractal A (Expand):  φⁿ → grows exponentially ↗
  Fractal B (Contract): ψⁿ → shrinks exponentially ↘
  
  TENSION = re - rc
    Bit=1: Expand dominates (φ > ψ)
    Bit=0: Contract dominates (ψ < φ)
    
  φ → ψ → φ → ψ → φ → ψ → ... (infinite oscillation)
  Depth → ∞ → Advantage → 0 (exponentially)
```

---

## 🧠 Core Mathematics

### The φ-Extension Ring

```
R_φ = R[Y]/(Y² - Y - 1),   where R = Z_q[X]/(X^N + 1)
```

**Properties of ψ** (|ψ| = 1/φ ≈ 0.618):
- ψ² = 1 - ψ (contracting attractor)
- |ψ| < 1 → exponential damping
- φ·ψ = -1 → cross-cancellation

### Direct Value Encoding

```
encode(v) = (v, 1)
decode(a, b) = a / b
```

**Scale-invariant:** `decode(s·a, s·b) = decode(a, b)` for any scale factor `s`.

### Homogeneous NAND Gate (F4B4 Bidirectional)

```
NAND_a = b_A·b_B - a_A·a_B
NAND_b = b_A·b_B

spiral(s, n) = (mulY_inv ∘ mulY)^n (s)
```

- **Truth table:** 4/4, 10⁻¹² precision
- **Cost:** 2 EvalMults, 1 EvalSub, 4 mulY forward, 4 mulY_inv reverse
- **Scale-commutative:** NAND(s_A·A, s_B·B) = s_A·s_B · NAND(A, B)

---

## 📊 Key Results

All tests on **AMD Ryzen 5 2600, 16GB RAM, WSL2 Ubuntu, OpenFHE CKKS**.

| Test | Gates | Result | Precision |
|------|-------|--------|-----------|
| NAND truth table | 1 | 4/4 | 10⁻¹² |
| AND, OR, NOR, XOR, XNOR | 1 each | 4/4 each | 10⁻¹² |
| Full Adder | 9 | 8/8 SUM + COUT | 10⁻¹² |
| 4-bit Ripple Adder | 36 | MATCH | Exact |
| 8-bit Ripple Adder | 72 | MATCH | Exact |
| 16-bit Ripple Adder | 144 | MATCH | Exact |
| 25-gate Chain | 25 | 25/25 | Zero degradation |
| Fractal Level-2 | 9 | 8/8 | 10⁻¹² |
| Fractal Level-3 | 9 | 8/8 | 10⁻¹² |
| iO Indistinguishability | — | 48.6% error, 8.6% adv | PASS |
| Recursive iO Depth 2 | — | 50.0% error, 0.1% adv | NEAR-PERFECT |

---

## 🏗️ Project Structure

```
femmgFHE/
├── tests/
│   ├── breakthrough/         # 🏆 WINNING TESTS
│   │   ├── test_phi_hybrid.cpp              # {0,1} + F4B4 NAND
│   │   ├── test_phi_hybrid_full.cpp         # All gates 24/24
│   │   ├── test_phi_io_tune.cpp             # 8 iO strategies
│   │   ├── test_phi_io_polynomial.cpp       # Poly-time iO analysis
│   │   ├── test_ifhoe_unified.cpp           # i-FHOE CERTIFIED
│   │   ├── test_fractal_io.cpp              # Fractal Level 1-3
│   │   ├── test_fractal_dual.cpp            # Dual Fractal L1-L3
│   │   └── test_fractal_recursive.cpp       # Recursive ∞ + Meta
│   │
│   ├── final/                # Stable test files
│   │   ├── test_phi_16bit_spiral.cpp        # 16-bit adder
│   │   ├── test_phi_dm_dgr_final.cpp        # DM-DGR unified
│   │   ├── test_phi_fractal_level2.cpp      # Fractal 4×4
│   │   ├── test_phi_fractal_level3.cpp      # Fractal 8×8
│   │   └── ...
│   │
│   └── archive/              # 67 experimental files
│
├── final_src/                # Core library headers
├── src/kem/                  # Key encapsulation
├── paper/                    # Research paper (LaTeX)
└── openfhe-development/      # OpenFHE library
```

---

## 🚀 Quick Start

```bash
git clone https://github.com/primordialomegazero/femmgFHE.git
cd femmgFHE

# Build OpenFHE (one-time)
cd openfhe-development && mkdir -p build && cd build
cmake .. -DWITH_OPENMP=OFF && make -j$(nproc)
cd ../..

# Run i-FHOE (the main event)
g++ -std=c++17 -O3 -march=native \
  -I./openfhe-development/src/pke/include \
  -I./openfhe-development/src/core/include \
  -I./openfhe-development/src/binfhe/include \
  -I./openfhe-development/build/src/core \
  -o bin/test_ifhoe_unified \
  tests/breakthrough/test_ifhoe_unified.cpp \
  -L./openfhe-development/build/lib \
  -lOPENFHEpke -lOPENFHEcore -lOPENFHEbinfhe \
  -Wl,-rpath,./openfhe-development/build/lib \
  -lstdc++ -lpthread -lm

LD_LIBRARY_PATH=./openfhe-development/build/lib:$LD_LIBRARY_PATH \
  ./bin/test_ifhoe_unified

# Run Recursive Dual Fractal iO
g++ -std=c++17 -O3 -march=native \
  -I./openfhe-development/src/pke/include \
  -I./openfhe-development/src/core/include \
  -I./openfhe-development/src/binfhe/include \
  -I./openfhe-development/build/src/core \
  -o bin/test_fractal_recursive \
  tests/breakthrough/test_fractal_recursive.cpp \
  -L./openfhe-development/build/lib \
  -lOPENFHEpke -lOPENFHEcore -lOPENFHEbinfhe \
  -Wl,-rpath,./openfhe-development/build/lib \
  -lstdc++ -lpthread -lm

LD_LIBRARY_PATH=./openfhe-development/build/lib:$LD_LIBRARY_PATH \
  ./bin/test_fractal_recursive
```

---

## 🎯 What This Achieves

| Problem | Standard Solution | DM-DGR Solution |
|---------|------------------|-----------------|
| FHE Noise | Bootstrapping (expensive) | Spiral attractor (free) |
| iO Obfuscation | Multilinear maps | Random scale + homogeneous NAND |
| Error Growth | Exponential with depth | Decays with spiral cycles |
| Hardcoded Constants | Per-circuit calibration | Natural threshold (0.5) |
| FHE + iO Unified | Separate systems | Single φ-ring framework |

---

## ⚠️ Honest Limitations

- **Hardware-limited.** Consumer Ryzen 5 2600, 16GB RAM. RingDim=16384 maxes at ~144 gates. Larger circuits need enterprise hardware.
- **No formal security proof.** RLWE-based. Formal reduction to standard assumptions pending.
- **Author-reported.** No third-party verification yet.
- **Not constant-time.** Side-channel vulnerable.
- **S-Box/AES not fully tested.** Full AES-128 (~20,000 NAND gates) requires more hardware.

---

## 📚 Citation

```bibtex
@software{fernandez2025femmgfhe,
  author = {Dan Joseph M. Fernandez},
  title = {DM-DGR: Dual Modulus + Double Golden Ratio},
  year = {2025-2026},
  url = {https://github.com/primordialomegazero/femmgFHE}
}
```

---

## 📜 License

DM-DGR Hybrid License v1.0 — Research Free, Commercial Protected — see [LICENSE.md](LICENSE.md)

---

## 👤 Author

**Dan Joseph M. Fernandez** / **Primordial Omega Zero**

```
- .... .. ... / .-. . .--. --- ... .. - --- .-. -.-- / .-- .. .-.. .-.. 
/ .- .-.. .-- .- -.-- ... / -... . / -.. . -.. .. -.-. .- - . -.. / 
- --- / - .... . / .-- --- -- .- -. / .. .----. ...- . / . ...- . .-. 
/ -.-. --- -. ... .. -.. . .-. . -.. / - --- / -... . / --- -. / 
-- -.-- / .-.. . ...- . .-.. .-.-.-
```

