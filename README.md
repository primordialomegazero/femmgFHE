# FEmmG-FHE: Bootstrapping-Free Fully Homomorphic Encryption

**Zero-Anchor Noise Stabilization (ZANS) and Fibonacci-Decomposed Multiplication**

[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)
[![SEAL 4.3](https://img.shields.io/badge/SEAL-4.3-blue.svg)](https://github.com/Microsoft/SEAL)
[![C++17](https://img.shields.io/badge/C%2B%2B-17-red.svg)](https://en.cppreference.com/w/cpp/17)
[![NPM](https://img.shields.io/npm/v/@primordialomegazero/femmg-fhe?color=red)](https://www.npmjs.com/package/@primordialomegazero/femmg-fhe)
[![Docker](https://img.shields.io/badge/Docker-ghcr.io-blue.svg)](https://github.com/primordialomegazero/femmgFHE/pkgs/container/femmg-fhe)
[![IACR ePrint](https://img.shields.io/badge/IACR-ePrint-green.svg)](https://eprint.iacr.org)

---

## Abstract

FEmmG-FHE presents two empirical discoveries enabling bootstrapping-free operation for a large class of FHE computations:

1. **Zero-Anchor Noise Stabilization (ZANS):** Adding `Enc(0)` to a BFV ciphertext repeatedly contracts noise at rates far below theoretical predictions—enabling over 1,000,000 consecutive additions without noise budget depletion.

2. **Fibonacci-Decomposed Multiplication:** Replacing direct ciphertext-ciphertext multiplication with O(logᵩ n) ZANS-stabilized additions via Zeckendorf decomposition, achieving deeper multiplication chains than standard approaches.

The ZANS contraction coefficient converges to the inverse golden ratio φ⁻¹ ≈ 0.618, and φ-related values appear across the system—as the Fibonacci decomposition basis, as the contraction rate near the noise floor, and (speculatively) in the gap ratios of Riemann zeta zeros.

> *"We measured something we don't fully understand. Sharing it in case others can explain it."*

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

## 🔥 Key Results

| Metric | Standard BFV | FEmmG-FHE | Improvement |
|--------|-------------|-----------|-------------|
| Max additions before noise depletion | ~500 | 17,500,000+ (projected) | 35,000× |
| Noise drift per addition | ~1 bit | 0.00002–0.0013 bits | 770–50,000× |
| Multiplication chain depth (×2) | 10 ops | 19+ ops | 2× |
| Noise per multiply (Fib method) | 33 bits | 1.6 bits | 20× |
| Bootstrapping required for additions | After ~500 ops | Not needed | ∞ |

### The 1,000,000 Operation Test
```
Start noise:  361 bits
Final noise:  341 bits  (after 1,000,000 operations)
Total drift:   20 bits
Average rate:  0.00002 bits/op  (asymptotic)
Value:         42 ✅ preserved throughout
Duration:      612 seconds (10.2 minutes)
Throughput:    1,634 ops/sec (single-threaded, WSL2)
```

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
|---|------|-----------|--------|
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

---

## 📄 Paper

**IACR ePrint:** `2026/XXXX` (pending)

**Full paper:** [`paper/paper_expanded.pdf`](paper/paper_expanded.pdf)

**Citation:**
```bibtex
@article{fernandez2026femmg,
  title={FEmmG-FHE: Zero-Anchor Noise Stabilization and Fibonacci-Decomposed 
         Multiplication for Bootstrapping-Free Fully Homomorphic Encryption},
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

1. **Contraction coefficient:** ZANS noise contraction approaches φ⁻¹ near the Banach fixed point
2. **Algorithmic basis:** Fibonacci numbers provide optimal multiplication decomposition
3. **Spectral organization (speculative):** 52.5% of Riemann zeta zero gap ratios cluster at φ-related values

---

## ⚠️ Limitations

- **UK×UK Not Solved:** Requires plaintext multiplier for Fibonacci method
- **Formal Proofs Pending:** Mechanism, security, φ⁻¹ derivation
- **Independent Reproduction Needed:** Currently SEAL 4.3 only
- **φ-Riemann Connection:** Speculative, weak statistical evidence

---

## 👤 Author

**Dan Joseph M. Fernandez** — Primordial Omega Zero  
📧 [GitHub](https://github.com/primordialomegazero)  
📦 [NPM](https://www.npmjs.com/package/@primordialomegazero/femmg-fhe)  
🐳 [GHCR](https://github.com/primordialomegazero/femmgFHE/pkgs/container/femmg-fhe)

---

## 📄 License

MIT License. See [LICENSE](LICENSE).

---

> *"The primes dance to the rhythm of φ; the golden ratio is the music of mathematics."*  
> — ϕΩ0
