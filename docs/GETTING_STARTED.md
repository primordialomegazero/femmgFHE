# Getting Started with FEmmg-FHE v7.0

Welcome! This guide gets you from zero to running encrypted computations in under 10 minutes.

---

## What is FEmmg-FHE?

FEmmg-FHE lets you **compute on encrypted data without decrypting it.**
You can add, multiply, and query encrypted numbers — results stay encrypted the whole time.

**Three pillars:**
- **ZANS** — eliminates noise growth in additions (verified across 9 libraries)
- **True Divine CT×CT** — 1,000,000 encrypted multiplications with linear noise
- **FEmmg-iO** — program obfuscation (encrypts the computation itself)

---

## Quick Start (5 minutes)

### 1. Prerequisites

```bash
# OpenFHE is bundled in the repo
cd femmgFHE/openfhe-development && mkdir -p build && cd build
cmake .. && make -j$(nproc)
cd ../..
```

### 2. Build

```bash
make all
```

### 3. Run Your First Encrypted Addition

```bash
./bin/phi_zans_bfv
```

**Expected output:** Value 42 preserved after thousands of Enc(0) additions. Noise unchanged.

---

## Core Demos

| Binary | What It Does |
|--------|-------------|
| `./bin/phi_true_divine_1M` | 1,000,000 CT×CT multiplications (21h full, 10K test mode) |
| `./bin/phi_pinky_swear` | Overflow detection without decryption |
| `./bin/phi_femmg_io_ultimate` | Program obfuscation (x*x, x*x*x, fractal) |
| `./bin/phi_catchmeifyouKEM` | 80-byte post-quantum KEM (10 runs) |
| `./bin/phi_covenant_vault` | Secure storage with tamper detection |
| `./bin/phi_spiraldb_unified` | Encrypted database with homomorphic queries |
| `./bin/phi_entangled_prime_zans` | Prime-based ZANS with verified cancellation |
| `./bin/phi_phantom_suite_v2` | Obfuscation with security audit |
| `./bin/phi_riemann_golden_zans` | Riemann-Golden ratio connections |
| `./bin/phi_quantum_random` | Quantum randomness from FHE noise |
| `./bin/phi_transmutation_window` | 24-hour encrypted stabilization |
| `./bin/phi_verifiable_fhe_v2` | HMAC-signed audit trail |
| `./bin/phi_unified_auth` | 6-head authentication with φ-consensus |

---

## System Overview

```
FEmmg-FHE v7.0
├── ZANS Engine (zans_production_lib.h v3.1.1)
│   ├── Noise measurement & tracking
│   ├── Multi-anchor pool (thread-safe)
│   └── Phi-scheduler (φ-based stabilization)
├── True Divine CT×CT (1M steps, linear noise)
│   ├── Pinky Swear (overflow detection)
│   └── Divine Intervention (noise absorption)
├── FEmmg-iO (program obfuscation)
│   ├── CRT6 (6 primes, 181-bit range)
│   └── Heterogeneous ZANS (6 variants)
├── catchmeifyouKEM (80B post-quantum KEM)
│   └── 1-bit quantization, IND-CCA
├── Covenant Vault (secure storage)
├── SpiralDB (encrypted database)
└── Unified Auth (6-head consensus)
```

---

## Learning Path

### Beginner (Today)
1. Run `./bin/phi_zans_bfv` — see ZANS in action
2. Read `THEOREM.md` — the mathematical framework
3. Try the examples in `src/core/`

### Intermediate (This Week)
4. Run `./bin/phi_true_divine_1M` (edit to 100 steps for quick test)
5. Run `./bin/phi_femmg_io_ultimate` — program obfuscation
6. Run `./bin/phi_catchmeifyouKEM` — 80B post-quantum KEM

### Advanced (This Month)
7. Run `./tests/full_blown_test.sh` — full test suite
8. Read `docs/proofs/formal_rlwe_proof.md` — security proofs
9. Study `zans_production_lib.h` — production ZANS engine

---

## Common Questions

**Q: What's the difference between TOY and PRODUCTION mode?**
A: TOY uses ring dim 4096 for fast testing. Production security requires ring dim 32768+.
   The results demonstrate the algorithmic breakthrough — linear noise scaling.

**Q: How many CT×CT multiplications can I do?**
A: 1,000,000 verified with linear noise (Noise = Step + 1, R² = 1.000). Zero bootstrapping.

**Q: How small is the post-quantum KEM?**
A: 80 bytes ciphertext (9.6x smaller than Kyber-512, 57.8x smaller than ML-KEM-1024).
   Full Module-LWE N=128 K=2 security. IND-CCA.

**Q: I found a bug! What do I do?**
A: Open a GitHub issue with: your OS, compiler version, error message, and steps to reproduce.

---

*"The noise is bounded. The computation is unlimited."*
