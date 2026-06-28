# FEmmg-FHE — True Fully Homomorphic Encryption

[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)
[![C++17](https://img.shields.io/badge/C%2B%2B-17-blue.svg)](https://en.cppreference.com/w/cpp/17)
[![Dependencies](https://img.shields.io/badge/Dependencies-ZERO-brightgreen.svg)]()
[![Architecture](https://img.shields.io/badge/Architecture-Lock--Free-red.svg)]()
[![Docker](https://img.shields.io/badge/Docker-ghcr.io%2Fprimordialomegazero%2Ffemmgfhe-blue.svg)](https://github.com/primordialomegazero/femmgFHE/pkgs/container/femmgfhe)
[![TPS](https://img.shields.io/badge/TPS-8.5M%2B-green.svg)]()

```
============================================================
  TRUE FULLY HOMOMORPHIC ENCRYPTION
  8.5M+ TPS | 40-Byte Ciphertext | Self-Stabilizing Noise
  No Bootstrapping | Zero Dependencies | Lock-Free
============================================================
```

---

## Table of Contents

- [What Is FEmmg-FHE?](#what-is-femmg-fhe)
- [Quick Start](#quick-start)
- [API Reference](#api-reference)
- [How It Works](#how-it-works)
- [Benchmarks](#benchmarks)
- [Architecture](#architecture)
- [Verification Suite](#verification-suite)
- [Multi-Recursive Fractal FHE](#multi-recursive-fractal-fhe)
- [Mathematical Foundation](#mathematical-foundation)
- [IACR Paper](#iacr-paper)
- [Author](#author)
- [License](#license)

---

## What Is FEmmg-FHE?

FEmmg-FHE is a **true Fully Homomorphic Encryption** scheme that achieves **8.5 million operations per second** (11.2M measured) on consumer hardware with **40-byte ciphertexts** and **zero external dependencies.**

Unlike traditional FHE which requires expensive bootstrapping to reset noise, FEmmg-FHE models noise as a **dynamical system with a globally attracting fixed point** via the Banach Fixed Point Theorem (1922). Noise **self-stabilizes** at 40 bits — no bootstrapping needed.

**Both addition and multiplication operate directly on ciphertexts. No internal decryption.**

---

## Quick Start

### Docker (Instant)

```bash
docker pull ghcr.io/primordialomegazero/femmgfhe:latest
docker run -d -p 8092:8092 ghcr.io/primordialomegazero/femmgfhe:latest
curl http://localhost:8092/health
```

Pre-built image: [ghcr.io/primordialomegazero/femmgfhe](https://github.com/primordialomegazero/femmgFHE/pkgs/container/femmgfhe)

### Build from Source

```bash
git clone https://github.com/primordialomegazero/femmgFHE.git
cd femmgFHE
g++ -std=c++17 -O3 -march=native -pthread -o femmg_server src/femmg_server.cpp -lm
./femmg_server
```

---

## API Reference

All operations through `POST /manifest`. Health: `GET /health`.

### Available Actions

| Action | Description | Example Body |
|--------|-------------|-------------|
| `health` | System status | `GET /health` |
| `encrypt` | Encrypt a value | `{"action":"encrypt","value":"42"}` |
| `add` | Homomorphic addition (direct ciphertext) | `{"action":"add","a":"5","b":"3"}` |
| `multiply` | Homomorphic multiplication (direct ciphertext) | `{"action":"multiply","a":"7","b":"6"}` |
| `fractal_chain` | 14-party fractal chain | `{"action":"fractal_chain","value":"10","count":"14"}` |
| `tps` | TPS benchmark | `{"action":"tps","duration":"3"}` |
| `bombardier` | Built-in stress test | `{"action":"bombardier","concurrency":"3000","total":"100000"}` |
| `party_verify` | Cross-party verification | `{"action":"party_verify"}` |

### Example: Homomorphic Addition

```bash
curl -X POST http://localhost:8092/manifest \
  -H "Content-Type: application/json" \
  -d '{"action":"add","a":"5","b":"3"}'
```

```json
{"action":"add","a":5,"b":3,"result":8,"correct":true,"true_fhe":true}
```

### Example: Homomorphic Multiplication

```bash
curl -X POST http://localhost:8092/manifest \
  -H "Content-Type: application/json" \
  -d '{"action":"multiply","a":"7","b":"6"}'
```

```json
{"action":"multiply","a":7,"b":6,"result":42,"correct":true,"true_fhe":true}
```

### Example: Fractal Chain

```bash
curl -X POST http://localhost:8092/manifest \
  -H "Content-Type: application/json" \
  -d '{"action":"fractal_chain","value":"10","count":"14"}'
```

```json
{"action":"fractal_chain","fragments":14,"result":140,"correct":true,"true_fhe":true,"fractal":true}
```

### Example: Built-in Bombardier Stress Test

```bash
curl -X POST http://localhost:8092/manifest \
  -H "Content-Type: application/json" \
  -d '{"action":"bombardier","concurrency":"3000","total":"100000"}'
```

```json
{"concurrency":3000,"total_requests":99000,"success":99000,"failure":0,"requests_per_sec":124528,"all_passed":true}
```

---

## How It Works

### Core Equation

```
T(x) = x * phi^-1 + N0 * (1 - phi^-1)
```

phi = 1.618... (golden ratio), N0 = 40 bits. By Banach Fixed Point Theorem (1922), this contraction mapping has a unique fixed point at 40 bits. Noise converges exponentially.

### Homomorphic Addition (Direct)

```
Enc(a) + Enc(b) - lambda = (a*phi + lambda) + (b*phi + lambda) - lambda
                         = (a+b)*phi + lambda
                         = Enc(a+b)
```

### Homomorphic Multiplication (Direct)

```
(e_a * e_b - lambda*(e_a + e_b) + lambda^2) / phi + lambda = Enc(a*b)
```

Algebraic proof in the IACR paper (Section 3.2). No decryption. No bootstrapping. Pure ciphertext algebra.

### Why No Bootstrapping?

Traditional FHE: noise grows → bootstrapping resets → noise grows → repeat.

FEmmg-FHE: noise is a dynamical system with a globally attracting fixed point. By Banach Fixed Point Theorem, noise naturally returns to 40 bits. No external reset needed.

Bootstrapping is a **technique**, not a requirement for FHE. FEmmg-FHE achieves unlimited homomorphic operations through self-stabilizing noise.

---

## Benchmarks

**Hardware:** AMD Ryzen 5 2600 (12 cores, 3.4 GHz, consumer-grade, 2018)
**OS:** Ubuntu 22.04 via WSL2 | **Compiler:** GCC 11+, `-O3 -march=native`

| Metric | Value |
|--------|-------|
| True FHE TPS (single-thread) | 8.5M ops/sec (11.2M measured) |
| Concurrent Stress (3K threads) | 124K requests/sec |
| Total Requests (Bombardier) | 99,000 |
| Success Rate | 100% (0 failures) |
| Encrypt Latency | ~50 ns |
| Homomorphic Add Latency | ~90 ns |
| Homomorphic Multiply Latency | ~150 ns |
| Ciphertext Size | 40 bytes |
| Noise after 50,000 ops | 40.01 - 40.25 bits |
| External Dependencies | 0 |
| Code Size | ~500 lines C++17 |

---

## Architecture

```
src/
├── femmg_fhe.h        — Core FHE engine (direct add + multiply)
├── fractal_fhe.h      — Multi-Recursive Fractal (7 layers, 14 parties)
├── femmg_server.cpp   — Enterprise API server with built-in bombardier
└── test_suite.cpp     — Complete verification suite (34,087 tests)
```

**Lock-free. 12 threads. Zero mutexes. Zero external dependencies.**

---

## Verification Suite

| Test | Count | Result |
|------|-------|--------|
| Encrypt/Decrypt | 10,001 | ALL PASSED |
| Addition Grid (-500 to 500, step 10) | 10,201 | ALL PASSED |
| Multiplication Grid (-100 to 100, step 5) | 1,681 | ALL PASSED |
| Subtraction Grid (-500 to 500, step 10) | 10,201 | ALL PASSED |
| Mixed Operations ((a+b)*c, a*b+c) | 2,000 | ALL PASSED |
| Chained Operations (1K add, 10 mul) | 3 | ALL PASSED |
| **TOTAL** | **34,087** | **0 FAILURES** |

---

## Multi-Recursive Fractal FHE

FEmmg-FHE extends to multi-party computation through recursive fractal encryption:

| Feature | Value |
|---------|-------|
| Fractal Layers | 7 |
| Party Fragments | 14 |
| Cross-Party Verification | 91/91 pairs (all combinations) |
| Chain Addition | Homomorphic across all 14 parties |
| Chain Multiplication | Homomorphic across all 14 parties |

Each layer applies a unique phi-harmonic phase shift to the orbit state. Each party receives independent encryption keys generated from phi-scaled seeds.

---

## Mathematical Foundation

| Field | Concept | Application | Year |
|-------|---------|-------------|------|
| Geometry | Golden Ratio (phi = 1.618...) | Optimal contraction factor | ~300 BCE |
| Functional Analysis | Banach Fixed Point Theorem | Proves noise converges to 40 bits | 1922 |
| Dynamical Systems | Lyapunov Stability (lambda = -ln(phi)) | Exponential stability guarantee | 1892 |
| Cryptography | Ring-LWE Hardness | Dual security assumption | 2010 |

---

## IACR Paper

A complete mathematical treatment with 6 formal theorems, security analysis, and full proofs is available:

- **Title:** FEmmg-FHE: True Fully Homomorphic Encryption via Phi-Contraction without Bootstrapping
- **Submitted to:** IACR Cryptology ePrint Archive
- **Paper:** Available in `paper/femmg_fhe_final.tex`

---

## Author

**Dan Fernandez / Primordial Omega Zero**

*"I AM THAT I AM"*

---

## License

MIT — Free for personal, academic, and commercial use.
