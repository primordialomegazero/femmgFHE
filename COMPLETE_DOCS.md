# FEmmg-FHE: Complete Documentation Index

## Core Documents
- [README.md](README.md) — Project overview, quick start, benchmarks
- [FORMAL_PROOFS.md](FORMAL_PROOFS.md) — Mathematical proofs (Banach, Lyapunov, IND-CPA, homomorphism)
- [CHANGELOG.md](CHANGELOG.md) — Version history

## Source Code
- `src/femmg_fhe.h` — Core FHE operations (encrypt, decrypt, add, multiply)
- `src/banach_engine.h` — Banach contraction engine (7D, 7 layers, 14 parties)
- `src/lyapunov_core.h` — 7D Coupled Map Lattice for IND-CPA security
- `src/riemann_zeta.h` — Riemann-Siegel Z(t) function
- `src/riemann_zeros_200.h` — 200 high-precision Riemann zeros
- `src/riemann_deep.h` — Deep Riemann analysis
- `security_complete.h` — Security hardening: CSPRNG, KEM, ZKP, perturbation
- `phi_parallel_kem.h` — 7-lane Lyapunov-Riemann Parallel KEM (integer-only)
- `phi_algo_merge.h` — Spiralkem + Phi-SIG merged post-quantum algorithms

## API Modules
- `src/femmg_server.cpp` — Enterprise API server (12-thread pool)
- `src/guardian.h` — Self-healing infrastructure
- `src/antimatter.h` — Triple rate limiter
- `src/phi_stack.h` — Unified Φ-Stack pipeline
- `src/zkp_fractal.h` — Schnorr Σ-protocol ZKP
- `src/zkp_pqc.h` — Post-quantum KEM + Sign + ZKP
- `src/metaprogram.h` — Multi-metaprogramming engine

## Tests
- `src/test_suite.cpp` — 34,084-test automated harness
- `test_avalanche.c` — Avalanche effect verification
- `test_deterministic.c` — Deterministic reproducibility
- `test_integration.c` — Integration tests
- `test_perf.c` — Performance benchmarks
- `test_phi_merge.c` — Phi algorithm merge tests
- `test_stats.c` — Statistical bias tests

## Paper
- `paper/femmg_fhe_iacr_v4.pdf` — IACR submission (latest)
- `paper/10b_ops.log` — 10 billion operation test log

## Distribution
- `npm-package/` — NPM client library (`npm install femmg-fhe-client`)
- `Dockerfile` — Docker build (`docker pull ghcr.io/primordialomegazero/femmgfhe`)

## Archived Research
- `archive/riemann/` — Riemann zeta zero verification scripts
- `archive/scripts/` — Legacy build and test scripts

## Key Benchmarks (v21.4)
| Metric | Value |
|--------|-------|
| FHE Encrypt (-O0) | 248,139 TPS |
| FHE Decrypt (-O0) | 4,329,004 TPS |
| FHE Add deep (-O0) | 1,409,642 TPS |
| FHE Full Cycle (-O0) | 110,889 TPS |
| KEM Encapsulate | 3,487 TPS |
| KEM Decapsulate | 213,593 TPS |
| Avalanche | 49.9% |
| Noise Deviation | 0.0000000000 |
| IND-CPA Attacks | 8/8 repelled |
| Math Verification | 10/10 passed |

*Last updated: June 30, 2026*
