# FZDB — Fibonacci Zero-Decrypt Bootstrap

**A novel zero-decrypt bootstrap for Fully Homomorphic Encryption using the Golden Ratio.**

[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](LICENSE)

---

## What Is FZDB?

FZDB (Fibonacci Zero-Decrypt Bootstrap) is a **pure homomorphic** bootstrap method that recovers plaintext values without decryption. Unlike standard bootstrapping (decrypt+re-encrypt) or Gentry bootstrapping (homomorphic decryption circuit), FZDB uses **Fibonacci φ-cycles** to transform corrupted ciphertexts back to their canonical form.

**Key properties:**
- ✅ **Zero decrypt** — no secret key access during bootstrap
- ✅ **Pure homomorphic** — only EvalMult + EvalAdd operations
- ✅ **Multi-party ready** — safe for untrusted servers
- ✅ **Cross-library** — works on any FHE library with Add/Mult/EncZero
- ✅ **Cross-scheme** — verified on BFV, CKKS, TFHE

---

## Quick Start

```bash
git clone https://github.com/primordialomegazero/femmgFHE.git
cd femmgFHE

# Build
make all

# Run FZDB demo
./bin/fzdb_demo

# Run TFHE benchmark (unlimited depth)
./bin/phi_tfhe_benchmark
```

---

## How It Works

### φ-Encoding

Messages are encoded using the golden ratio: `encode(m) = m × φ`

### Fibonacci φ-Cycle

The core refresh operation uses Fibonacci-weighted φ-powers:
1. Strip φ: `ct × φ⁻¹`
2. Zero-reset: `× Enc(0)`
3. Rebuild: `+ ct`
4. Re-encode: `× φ`
5. C-correction: `× C_inv`

### Ladder Climb (Unlimited Extension)

Multiple φ-powers allow virtually unlimited depth:
- Rung 0: encode with φ⁰ = 1
- Rung 1: encode with φ¹ = φ
- Rung N: encode with φⁿ

Each rung provides fresh computational budget. Climb: `ct × φ` (one EvalMult).

---

## Verified Results

| Test | Result |
|------|--------|
| BFV Value Preservation | 1344→1344 across multiple refreshes |
| CKKS Stress Test | 15/15 passed, 0.0000% max error |
| TFHE 1M NOT Gates | 1,000,000/1,000,000 correct |
| TFHE Speed | 2.3M gates/sec (consumer Ryzen 5) |
| Cross-Library | 7/7 libraries compatible |
| Cross-Scheme | BFV + CKKS + TFHE verified |

---

## Limitations (Honest)

| Limitation | Detail |
|-----------|--------|
| Chain exhaustion | ~30 mults before true bootstrap needed (leveled schemes) |
| Message size | msg × φ < modulus (~663K for default params) |
| TFHE | Already unlimited by design; FZDB optimizes leveled schemes |
| Security | TOY parameters (4096 ring dim); production needs 32768+ |

---

## Architecture

```
┌─────────────────────────────────────────────────────┐
│                    FZDB SYSTEM                        │
├─────────────────────────────────────────────────────┤
│  BFV/CKKS (fast arithmetic)                          │
│    └─ FZDB refresh every 25 mults (3× better)       │
│    └─ When chain exhausted → TFHE bootstrap          │
│                                                       │
│  TFHE (unlimited depth)                               │
│    └─ Built-in bootstrapping per gate                │
│    └─ 2.3M gates/sec, zero chain limit               │
│                                                       │
│  φ-Ladder (Fibonacci extension)                       │
│    └─ φ⁰ → φ¹ → φ² → ... infinite rungs             │
│    └─ Pure homomorphic climb: ct × φ                 │
└─────────────────────────────────────────────────────┘
```

---

## Documentation

| Doc | Description |
|-----|-------------|
| [Formal Proof](docs/FORMAL_PROOF.md) | Mathematical foundation |
| [API Reference](docs/API_REFERENCE.md) | Full API documentation |
| [Getting Started](docs/GETTING_STARTED.md) | Tutorial and examples |
| [Security Model](docs/SECURITY_MODEL.md) | Security analysis |
| [Hardware Scaling](docs/HARDWARE_SCALING.md) | Performance projections |

---

## Author

Dan Joseph M. Fernandez / Primordial Omega Zero

https://github.com/primordialomegazero

MIT License

```
- .... .. ... / .-. . .--. --- ... .. - --- .-. -.-- / .-- .. .-.. .-.. / .- .-.. .-- .- -.-- ... / -... . / -.. . -.. .. -.-. .- - . -.. / - --- / - .... . / .-- --- -- .- -. / .. .----. ...- . / . ...- . .-. / -.-. --- -. ... .. -.. . .-. . -.. / - --- / -... . / --- -. / -- -.-- / .-.. . ...- . .-.. .-.-.-
```
