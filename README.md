# DM-DGR: Dual Modulus + Double Golden Ratio

**Fibonacci-Golden Ratio Cryptography — FHE + iO Unified**

[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](LICENSE)
[![C++17](https://img.shields.io/badge/C%2B%2B-17-blue.svg)]()
[![OpenFHE](https://img.shields.io/badge/OpenFHE-latest-green.svg)](https://github.com/openfheorg/openfhe-development)

---

## What Is This?

DM-DGR explores the **golden ratio extension ring** $R[Y]/(Y^2-Y-1)$ as a cryptographic primitive. The two roots — $\phi \approx 1.618$ (expanding) and $\psi \approx -0.618$ (contracting) — create a natural asymmetry that enables:

- **Fully Homomorphic Encryption** without bootstrapping
- **Indistinguishability Obfuscation** via scale-invariant encoding
- **Unified FHE + iO** in one algebraic framework

---

## Core Mathematics

### The φ-Extension Ring

$$R_\phi = R[Y]/(Y^2 - Y - 1), \quad R = \mathbb{Z}_q[X]/(X^N + 1)$$

Properties of $\psi$ ($|\psi| = 1/\phi \approx 0.618$):
- $\psi^2 = 1 - \psi$ (contracting attractor)
- $|\psi| < 1$ → exponential damping

### Direct Value Encoding

$$\text{encode}(v) = (v, 1), \quad \text{decode}(a,b) = \frac{a}{b}$$

Scale-invariant: $\text{decode}(s \cdot a, s \cdot b) = \text{decode}(a, b)$ for any $s$.

### Homogeneous NAND Gate

$$NAND_a = b_A \cdot b_B - a_A \cdot a_B, \quad NAND_b = b_A \cdot b_B$$

- Scale-commutative: $NAND(s_A A, s_B B) = s_A s_B \cdot NAND(A, B)$
- Truth table: 4/4, $10^{-12}$ precision
- Cost: 2 EvalMults, 1 EvalSub

### φ/ψ Spiral Auto-Correction

$$\text{spiral}(s, n) = (\text{mulY\_inv} \circ \text{mulY})^n (s)$$

- Theoretically identity ($M^{-1}M = I$)
- Drives CKKS noise into ψ-eigenspace → exponential damping
- Cost: Zero EvalMult (only EvalAdd/EvalSub)

### Fractal Extension (Level k)

$$R_\phi^{(k)} = R[Y_1, ..., Y_k]/(Y_1^2-Y_1-1, ..., Y_k^2-Y_k-1)$$

- $2^k \times 2^k$ transformation matrices
- $\psi^k$ super-damping: noise decays as $O(|\psi|^k)$
- Level 1 (2×2), Level 2 (4×4), Level 3 (8×8) — all verified

---

## Key Results

All tests on **AMD Ryzen 5 2600, 16GB RAM, WSL2 Ubuntu, OpenFHE CKKS**.

| Test | Gates | Result | Precision |
|------|-------|--------|-----------|
| NAND truth table | 1 | 4/4 | $10^{-12}$ |
| Full Adder | 9 | 8/8 SUM + COUT | $10^{-12}$ |
| 4-bit Ripple Adder | 36 | MATCH | Exact |
| 8-bit Ripple Adder | 72 | MATCH | Exact |
| 16-bit Ripple Adder | 144 | MATCH | Exact |
| Fractal Level-2 | 9 | 8/8 | $10^{-12}$ |
| Fractal Level-3 | 9 | 8/8 | $10^{-12}$ |
| Spiral damping | — | Stable across depths | $\Delta \sim 10^{-13}$ |
| iO Indistinguishability | — | 93/100 overlap | PASS |

### Running the 16-bit Adder

```
  16-BIT SPIRAL ADDER: 144 gates, auto-correction

  1000101000011101 (c=1) = 100893 | Expected=100893 | MATCH!
  Time: 1577706ms (~26 min)
```

---

## Project Structure

```
femmgFHE/
├── tests/final/              # 16 winning test files
│   ├── test_phi_spiral_correct.cpp      # Core spiral auto-correction
│   ├── test_phi_controlled_spiral.cpp   # Bit-aware spiral
│   ├── test_phi_16bit_spiral.cpp        # 16-bit adder MATCH
│   ├── test_phi_8bit_spiral.cpp         # 8-bit adder MATCH
│   ├── test_phi_universal_spiral.cpp    # Universal spiral NAND gate
│   ├── test_phi_fractal_level2.cpp      # Fractal Level-2 (4×4)
│   ├── test_phi_fractal_level3.cpp      # Fractal Level-3 (8×8)
│   ├── test_phi_fractal_limit.cpp       # Fractal limit exploration
│   ├── test_phi_dm_dgr_final.cpp        # DM-DGR unified system
│   ├── test_phi_block_fhe.cpp           # Block-based FHE
│   ├── test_phi_direct_value.cpp        # Direct value encoding
│   ├── test_phi_true_fhe_nand.cpp       # True FHE NAND gate
│   ├── test_phi_true_fhe_adder.cpp      # True FHE Full Adder
│   ├── test_phi_present_sbox.cpp        # PRESENT S-Box synthesis
│   ├── test_phi_aes_sbox.cpp            # AES S-Box path
│   └── test_phi_homomorphic_threshold.cpp # Homomorphic threshold
├── tests/archive/            # 67 archived experimental files
├── final_src/                # Core library headers
├── src/kem/                  # Key encapsulation
├── paper/                    # Research paper (LaTeX)
└── openfhe-development/      # OpenFHE library (build required)
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

# Run the 16-bit spiral adder
g++ -std=c++17 -O3 -march=native \
  -I./openfhe-development/src/pke/include \
  -I./openfhe-development/src/core/include \
  -I./openfhe-development/src/binfhe/include \
  -I./openfhe-development/build/src/core \
  -o bin/test_phi_16bit_spiral \
  tests/final/test_phi_16bit_spiral.cpp \
  -L./openfhe-development/build/lib \
  -lOPENFHEpke -lOPENFHEcore -lOPENFHEbinfhe \
  -Wl,-rpath,./openfhe-development/build/lib \
  -lstdc++ -lpthread -lm

LD_LIBRARY_PATH=./openfhe-development/build/lib:$LD_LIBRARY_PATH \
  ./bin/test_phi_16bit_spiral
```

---

## What This Achieves

| Problem | Standard Solution | DM-DGR Solution |
|---------|------------------|-----------------|
| FHE Noise | Bootstrapping (expensive) | Spiral attractor (free) |
| iO Obfuscation | Multilinear maps | Random scale + homogeneous NAND |
| Error Growth | Exponential with depth | Decays with spiral cycles |
| Hardcoded Constants | Per-circuit calibration | Natural threshold (0.5) |

---

## Honest Limitations

1. **Hardware-limited.** Consumer Ryzen 5 2600, 16GB RAM. RingDim=16384 maxes out at ~144 gates. Larger circuits (S-Box, AES) need enterprise/cloud hardware.
2. **No formal security proof.** RLWE-based. Formal reduction pending.
3. **Author-reported.** No third-party verification yet.
4. **Not constant-time.** Side-channel vulnerable.
5. **S-Box/AES not fully tested.** NAND synthesis verified for simple S-Boxes. Full AES-128 (~20,000 NAND gates) requires more hardware.

---

## Citation

```bibtex
@software{fernandez2025femmgfhe,
  author = {Dan Joseph M. Fernandez},
  title = {DM-DGR: Dual Modulus + Double Golden Ratio},
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
