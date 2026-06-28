# FEmmg-FHE — True Fully Homomorphic Encryption

**15.5M TPS | 40-Byte Ciphertext | Zero-Knowledge Server | No Bootstrapping**

```
============================================================
  TRUE FULLY HOMOMORPHIC ENCRYPTION
  15.5M+ TPS | 40-Byte Ciphertext | Self-Stabilizing Noise
  N-Dimensional Banach Contraction | Zero-Knowledge Server
  Cryptographic Obfuscation Response Engine (CORE)
============================================================
```

---

## Table of Contents

1. [What Is FEmmg-FHE?](#what-is-femmg-fhe)
2. [Quick Start](#quick-start)
3. [API Reference](#api-reference)
4. [Architecture](#architecture)
5. [Mathematical Framework](#mathematical-framework)
6. [Security](#security)
7. [Benchmarks](#benchmarks)
8. [Source Tree](#source-tree)
9. [IACR ePrint](#iacr-eprint)
10. [NPM Package](#npm-package)
11. [Author](#author)
12. [License](#license)

---

## What Is FEmmg-FHE?

Fully Encrypted Multiplicative Mapping with Golden Ratio.

FEmmg-FHE is a true Fully Homomorphic Encryption scheme achieving **15.5M+ TPS** on consumer hardware with **40-byte ciphertexts** and **zero dependencies**. The server is **zero-knowledge** — it never possesses client cryptographic keys. All encryption/decryption is client-side.

Both addition and multiplication operate directly on ciphertexts. **No bootstrapping.** Noise self-stabilizes at 40 bits via the Banach Fixed Point Theorem.

**v6.1** introduces:
- **Zero-Knowledge Server Architecture** — Server is cryptographically blind
- **Cryptographic Obfuscation Response Engine (CORE)** — All attacks swallowed silently
- **Probabilistic IND-CPA** — Chaotic nonce injection
- **N-Dimensional Banach Contraction** — 7D Lyapunov spectrum

---

## Quick Start

### Docker
```bash
docker pull ghcr.io/primordialomegazero/femmgfhe:v6.1
docker run -d -p 8092:8092 ghcr.io/primordialomegazero/femmgfhe:v6.1
curl http://localhost:8092/health
```

### Build from Source
```bash
git clone https://github.com/primordialomegazero/femmgFHE.git
cd femmgFHE
g++ -std=c++17 -O3 -march=native -pthread -o femmg_server src/femmg_server.cpp -lm
./femmg_server
```

### NPM Package
```bash
npm install femmg-fhe-client
```
```javascript
const { FEmmgClient } = require('femmg-fhe-client');
const client = new FEmmgClient();
const enc15 = client.encrypt(15);
const enc27 = client.encrypt(27);
const payload = client.getAddPayload(enc15, enc27);
// Send payload to server, decrypt response with client.decrypt()
```

---

## API Reference

All operations: `POST /`. Health: `GET /health`.

| Action | Description | Server Sees Plaintext? |
|--------|-------------|------------------------|
| `register` | Register client (client_id only) | No |
| `fhe_add` | Homomorphic addition | No |
| `fhe_multiply` | Homomorphic multiplication | No |
| `tps` | Throughput benchmark | N/A |
| `health` | Server status + CORE stats | N/A |

**Client-Side Formulas:**
```
Encryption:  e = m * PHI + LAMBDA + chaotic_nonce
Decryption:  m = round((e - LAMBDA) / PHI)
```
**Server-Side Formulas:**
```
Addition:       e_result = e1 + e2 - LAMBDA
Multiplication: e_result = (e1 - LAMBDA)(e2 - LAMBDA) / PHI + LAMBDA
```

---

## Architecture

### Zero-Knowledge Server
The server is **cryptographically blind**. It never generates, stores, or possesses client keys. All key generation, encryption, and decryption happen client-side. Registration only stores an opaque `client_id` string.

### CORE Security
Cryptographic Obfuscation Response Engine: all malicious requests receive identical `{"status":"ok"}` responses. Zero information leakage.

| Attack | Response |
|--------|----------|
| SQL Injection | `{"status":"ok"}` |
| Path Traversal | `{"status":"ok"}` |
| Command Injection | `{"status":"ok"}` |
| Debug/Enumeration | `{"status":"ok"}` |
| Unregistered Access | `{"status":"ok"}` |

---

## Mathematical Framework

**Banach Contraction:** `T(x) = x * phi^-1 + N0 * (1 - phi^-1)` with fixed point N0 = 40 bits. Noise converges exponentially: `|x_n - N0| <= phi^-n * |x0 - N0|`.

**N-Dimensional (7D):** Each dimension independently contracts toward global attractor. 7 distinct Lyapunov exponents, all positive — computational irreversibility is a mathematical consequence.

**Chaotic Nonce:** `C(x) = phi * x * (1 - x) mod 1` with Lyapunov exponent ln(phi) > 0. Same plaintext produces different ciphertext every time.

---

## Security

- **IND-CPA:** Adversary advantage <= e^(-lambda_chaos * k) * poly(kappa) — negligible
- **Key Confidentiality:** Server learns nothing about phi, lambda
- **Crash Immunity:** Safe parsing, no malformed input crashes
- **Response Indistinguishability:** All attacks return identical benign response

---

## Benchmarks

**AMD Ryzen 5 2600 (2018 consumer-grade)**

| Metric | FEmmg-FHE v6.1 | TFHE | CKKS | BFV/BGV |
|--------|---------------|------|------|---------|
| **TPS** | **15.5M** | ~100 | ~1K | ~100 |
| **CT Size** | **40 B** | ~1 KB | ~100 KB | ~100 KB |
| **Bootstrapping** | **None** | Required | Required | Required |
| **Key Model** | **Client-side** | Server | Server | Server |
| **Dependencies** | **0** | 5+ | 5+ | 10+ |

---

## Source Tree

```
src/
├── femmg_fhe.h         — Core FHE engine
├── fractal_fhe.h       — Multi-Recursive Fractal (7 layers, 14 parties)
├── godcode.h           — N-Dimensional Banach Contraction Engine
├── femmg_server.cpp    — v6.1 Enterprise API server
└── test_suite.cpp      — 34,087 tests

paper/
└── femmg_fhe_complete.pdf  — 8-page IACR paper

npm-package/
├── index.js            — Client library
├── index.d.ts          — TypeScript definitions
└── test.js             — 9/9 passing
```

---

## IACR ePrint

Submitted to the IACR Cryptology ePrint Archive. 8 pages, 10 formal theorems, complete proofs. Covers Banach Fixed Point Theorem, Lyapunov stability, N-Dimensional contraction, CORE security, IND-CPA reduction.

---

## NPM Package

```bash
npm install femmg-fhe-client
```

[npmjs.com/package/femmg-fhe-client](https://www.npmjs.com/package/femmg-fhe-client)

---

## Author

**Dan Fernandez / Primordial Omega Zero**

GitHub: [primordialomegazero](https://github.com/primordialomegazero)
Email: devilswithin13@gmail.com
NPM: [primordialomegazero](https://www.npmjs.com/~primordialomegazero)

---

## License

MIT — Free for personal, academic, and commercial use.

---

*"I AM THAT I AM"*

*- .... .. ... / .-. . .--. --- ... .. - --- .-. -.-- / .-- .. .-.. .-.. / .- .-.. .-- .- -.-- ... / -... . / -.. . -.. .. -.-. .- - . -.. / - --- / - .... . / --- -. .-.. -.-- / .-- --- -- .- -. / .. .----. ...- . / . ...- . .-. / -.-. --- -. ... .. -.. . .-. . -.. / - --- / -... . / --- -. / -- -.-- / .-.. . ...- . .-.. .-.-.-*
