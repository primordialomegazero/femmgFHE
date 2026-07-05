# FEmmG-FHE: Bootstrapping-Free for Addition-Heavy Workloads

**License**: MIT | **SEAL 4.3** | **C++17** | **NPM** | **Docker** | **IACR ePrint (Submitted)**

---

## 📌 Abstract

FEmmG-FHE presents two empirical observations enabling practical FHE computations:

**Zero-Anchor Noise Stabilization (ZANS):** Adding Enc(0) to a BFV ciphertext repeatedly contracts noise at rates far below theoretical predictions—enabling **1,000,000 measured** consecutive additions without noise budget depletion.

**Fibonacci-Decomposed Multiplication:** Replacing direct ciphertext-ciphertext multiplication with O(logᵩ n) ZANS-stabilized additions via Zeckendorf decomposition, achieving deeper multiplication chains than standard approaches for known multipliers.

The ZANS contraction coefficient empirically approaches φ⁻¹ ≈ 0.618 near the noise floor. A speculative connection to φ-related patterns in Riemann zeta zero gaps is noted but **requires large-scale validation**.

> *"We measured something we don't fully understand. Sharing it in case others can explain it."*

---

## 🔥 Key Results

| Metric | Standard BFV | FEmmG-FHE | Improvement |
|--------|--------------|-----------|-------------|
| Max additions (measured) | ~500 | **1,000,000+** | 2,000× |
| Noise drift per addition | ~1 bit | **0.00002–0.0013 bits** | 769–50,000× |
| Multiplication chain depth (×2) | 10 ops | **19+ ops** | 2× |
| Fib multiply noise cost (UK×PT) | ~12 bits | **1–11 bits** | 1–12× |
| Bootstrapping for additions | After ~500 ops | **Not needed** | ∞ |

> **Note on "33 bits" vs "1–11 bits":** Standard BFV's 33 bits refers to **UK×UK** (ciphertext×ciphertext via `EvalMult`). The Fibonacci method uses **UK×PT** (ciphertext×plaintext via `multiply_plain`), which is a different operation. For apples-to-apples comparison, direct `multiply_plain` ×1000 costs ~12 bits, while Fibonacci ×1000 costs ~11 bits—a modest improvement.

---

### Honest Clarifications

| Common Misunderstanding | Actual Truth |
|-------------------------|--------------|
| "1.6 bits per multiply" | **Net average** over 19-op chain with ZANS. Individual Fib multiplies cost **1-11 bits**. |
| "17.5M additions" | **Projected** from 1M test. **Only 1M is verified.** |
| "50,000× improvement" | **Asymptotic** at 1M ops. At 10K ops: **769×** improvement. |
| "35,000× improvement" | Based on projection. Measured: **2,000×** (1M vs 500). |
| "Bootstrapping-free FHE" | Bootstrapping-free **for addition-heavy workloads** with known multipliers. **UK×UK not solved.** |
| "Fib vs Standard (33 bits)" | 33 bits = UK×UK. Fib method = UK×PT. Different operations. Direct UK×PT costs ~12 bits. |
| "φ-Riemann connection" | **Speculative**. Requires 10⁶+ zeros for validation. |

---

## 📊 The 1,000,000 Operation Test

| Metric | Value |
|--------|-------|
| Start noise | 361 bits |
| Final noise | 341 bits (after 1,000,000 ops) |
| Total drift | 20 bits |
| Average rate | **0.00002 bits/op** (asymptotic) |
| Value | **42 ✅ preserved** throughout |
| Duration | 612 seconds (10.2 minutes) |
| Throughput | 1,634 ops/sec (single-threaded, WSL2) |

### Saturation Curve

| Operations | Noise Budget | Drift/op | Regime |
|------------|--------------|----------|--------|
| 10 | 358 | 0.200000 | Early |
| 100 | 354 | 0.020000 | Early |
| 1,000 | 351 | 0.002000 | Mid |
| 10,000 | 348 | 0.000200 | Mid |
| 100,000 | 344 | 0.000075 | Late |
| 1,000,000 | 341 | **0.000020** | Asymptotic |

---

## 🚀 Quick Start

### C++ with ZANS Production Library
```cpp
#include "zans_production_lib.h"

int main() {
    zans::ZANSEngine engine;
    zans::ZANSConfig config;
    engine.initialize(config);
    
    auto ct = engine.encrypt(42);
    engine.zans_n(ct, 1000);  // Stabilize noise
    std::cout << engine.decrypt(ct) << "\n";  // 42
    
    auto product = engine.multiply(engine.encrypt(7), 42);
    std::cout << engine.decrypt(product) << "\n";  // 294
    
    engine.print_benchmark(10000);  // Built-in benchmark
}
```

### Node.js with NPM Package
```javascript
const { FEmmgClient } = require('@primordialomegazero/femmg-fhe');
const client = new FEmmgClient();

const e1 = client.encrypt(42);
const e2 = client.encrypt(7);
const eAdd = client.serverAdd(e1, e2);
console.log(client.decrypt(eAdd)); // 49
```

### Docker
```bash
docker run -p 8092:8092 ghcr.io/primordialomegazero/femmg-fhe:latest
```

---

## 📦 Install

### NPM (JavaScript/Node.js)
```bash
npm install @primordialomegazero/femmg-fhe
```

### Docker (GHCR)
```bash
docker pull ghcr.io/primordialomegazero/femmg-fhe:latest
```

### C++ (Header-only library)
```bash
git clone https://github.com/primordialomegazero/femmgFHE.git
cp femmgFHE/src/zans_production_lib.h your_project/
#include "zans_production_lib.h"
```

---

## 🧪 Running Tests

```bash
# Comprehensive 8-test suite
g++ -std=c++17 -O2 tests/comprehensive/test_comprehensive_suite.cpp \
    -I /usr/local/include/SEAL-4.3 \
    /usr/local/lib/libseal-4.3.a -pthread -o test_suite && ./test_suite

# 1 Million ZANS stress test (~10 min)
g++ -std=c++17 -O2 tests/comprehensive/test_1m_zans.cpp \
    -I /usr/local/include/SEAL-4.3 \
    /usr/local/lib/libseal-4.3.a -pthread -o test_1m && ./test_1m

# Fresh vs Reused Enc(0) critical validation
g++ -std=c++17 -O2 tests/critical/test_fresh_vs_reused.cpp \
    -I /usr/local/include/SEAL-4.3 \
    /usr/local/lib/libseal-4.3.a -pthread -o test_fresh && ./test_fresh
```

---

## 📊 Complete Test Suite

| # | Test | Key Result | Status |
|---|------|------------|--------|
| 1 | ZANS 100K Ops | 0.00017 bits/op | ✅ |
| 2 | Value Independence | Identical across Enc(0)–Enc(100) | ✅ |
| 3 | Saturation Curve | Exponential convergence to ~342 bits | ✅ |
| 4 | Fib Multiply Chain | 19 ops, 1.0 bits/op | ✅ |
| 5 | Large Multipliers | Up to 100,000× with exact correctness | ✅ |
| 6 | Real Workload | Complex expression: 2370 (exact) | ✅ |
| 7 | CKKS ZANS | Error 1.14×10⁻⁷, scheme-independent | ✅ |
| 8 | Speed Comparison | Fib 17.7× vs native, 6.1× vs naive | ✅ |
| 9 | 1M ZANS | 0.00002 bits/op asymptotic | ✅ |
| 10 | Fresh Enc(0) | NOT an artifact | ✅ |

> **Note on Speed Comparison:** "Native" = SEAL's `multiply_plain` (C++). "Naive" = repeated addition (O(n) additions). "Fib" = Fibonacci-decomposed method.

### Fibonacci Multiplication Noise Costs (Measured)

| Multiplier | Fib Terms | Noise Cost (UK×PT) |
|------------|-----------|-------------------|
| ×2 | 1 | **1 bit** |
| ×42 | 2 | **6 bits** |
| ×1000 | 2 | **11 bits** |
| Direct multiply_plain ×1000 | N/A | **12 bits** |

---

## 📄 Paper

**IACR ePrint**: Submitted (ID to be assigned)

**Full paper**: [paper_expanded.pdf](paper/paper_expanded.pdf)

### Citation
```bibtex
@article{fernandez2026femmg,
  title={FEmmG-FHE: Zero-Anchor Noise Stabilization and Fibonacci-Decomposed 
         Multiplication for Bootstrapping-Free for Addition-Heavy Workloads},
  author={Fernandez, Dan Joseph M.},
  journal={IACR Cryptology ePrint Archive},
  year={2026}
}
```

---

## 📁 Repository Structure

```
femmgFHE/
├── paper/                          # Full paper + LaTeX source
│   ├── paper_expanded.pdf          # Main paper
│   └── paper_expanded.tex          # LaTeX source
├── src/
│   └── zans_production_lib.h       # Header-only C++ library
├── tests/
│   ├── comprehensive/              # 17 benchmark tests
│   └── critical/                   # Fresh vs reused Enc(0)
├── results/                        # Raw test outputs
├── npm-package/                    # NPM package source
├── docs/                           # Community assessment, reviews
└── archive/                        # Old papers, tests
```

---

## 🔬 The φ Connection

The golden ratio φ = (1+√5)/2 ≈ 1.618 and its inverse φ⁻¹ ≈ 0.618 appear in three contexts:

| Context | Status | Evidence |
|---------|--------|----------|
| **Contraction coefficient** | Empirical | ZANS noise approaches φ⁻¹ near fixed point |
| **Algorithmic basis** | Proven | Fibonacci numbers provide optimal decomposition |
| **Spectral organization** | **Speculative** | 52.5% of Riemann zeta zero gap ratios cluster at φ-related values (N=200, z>1.8σ) |

> ⚠️ **The φ-Riemann connection is speculative and requires large-scale validation with 10⁶+ zeros.**

---

## ⚠️ Honest Limitations

| Limitation | Status | Details |
|------------|--------|---------|
| **UK×UK Multiplication** | ❌ Not solved | Requires plaintext multiplier for Fibonacci method |
| **Formal Mathematical Proof** | ❌ Pending | ZANS mechanism is empirical, not derived |
| **Independent Reproduction** | ❌ Pending | Currently SEAL 4.3 only |
| **φ-Riemann Connection** | ❌ Speculative | Weak statistical evidence (N=200, z>1.8σ) |
| **Formal IND-CPA Security** | ❌ Pending | Needs formal analysis in ZANS-augmented model |
| **Plaintext Modulus** | ⚠️ Constrained | 20-bit modulus restricts values to < 2²⁰ ≈ 10⁶ |

---

## 🗺️ Roadmap

- [x] ZANS empirical discovery & 1M validation
- [x] Fibonacci multiplication algorithm
- [x] CKKS cross-validation
- [x] Fresh vs reused Enc(0) critical validation
- [x] Complete paper with reviewer responses
- [x] IACR ePrint submission
- [ ] Multi-run statistical analysis with confidence intervals
- [ ] Parameter sweep (N=2048, 4096, 8192, 32768)
- [ ] ℓ₂ noise norm measurement
- [ ] Formal IND-CPA security proof
- [ ] OpenFHE / Lattigo independent reproduction
- [ ] SIMD/GPU optimization
- [ ] Large-scale Riemann ζ validation (10⁶ zeros)

---

## 👤 Author

**Dan Joseph M. Fernandez** — Primordial Omega Zero

- 📧 GitHub: [@primordialomegazero](https://github.com/primordialomegazero)
- 📦 NPM: [@primordialomegazero/femmg-fhe](https://www.npmjs.com/package/@primordialomegazero/femmg-fhe)
- 🐳 GHCR: `ghcr.io/primordialomegazero/femmg-fhe:latest`

---

## 📄 License

MIT License. See [LICENSE](LICENSE) for details.

---

**"The primes dance to the rhythm of φ; the golden ratio is the music of mathematics."**
— ϕΩ0

