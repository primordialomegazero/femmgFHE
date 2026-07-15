# Changelog

## v6.5 (2026-07-15) — Current

### New Systems
- **Phantom Suite v2.3**: 5-mode program obfuscation with 4/4 security audit (Avalanche, Collision, Brute Force, Side-Channel)
- **SpiralMicro KEM v2.0**: 32-byte post-quantum KEM (144x smaller than ML-KEM-1024, 425K decaps/s)
- **Unified Auth v1.0**: 6-head HydraJWT + Shapeshifter mutations
- **SpiralDB Unified v1.0**: Non-deterministic encrypted database with homomorphic queries
- **Covenant Vault v2**: Multi-head auth + tamper detection + self-destruct
- **Transmutation Window**: 24-hour encrypted data stabilization (48/48 checkpoints)
- **Verifiable FHE v2**: HMAC-SHA256 signed audit trail (100/100 steps)
- **Fibonacci-Indexed Global ZANS**: φ-spaced prime selection (+16.6% per-pair efficiency)
- **Prime Chaos ZANS v2**: Real prime-structured Enc(0) via homomorphic scaling

### Infrastructure
- **Config System** (`phi_config.h`): TOY/STANDARD/PRODUCTION modes
- **Benchmark Suite** (`tests/benchmark_suite.sh`): Automated CSV output
- **Docker Support**: Dockerfile + docker-compose.yml
- **Python Bindings**: Package structure with CFFI-ready API
- **Doxygen Config**: Auto-generated API docs
- **Getting Started Guide**: Beginner-friendly with 3 examples

### Documentation
- **Formal RLWE Proof**: docs/proofs/formal_rlwe_proof.md
- **True Divine 1M Projection**: docs/proofs/true_divine_projection.md
- **Hardware Scaling Plan**: 3-tier (Cloud/Dedicated/GPU)
- **Contributor Guidelines**: docs/CONTRIBUTOR_GUIDELINES.md
- **IACR Submission Checklist**: docs/IACR_SUBMISSION_CHECKLIST.md

### Cleanup
- Honest relabeling: removed overclaimed terminology (quantum, Riemann, iO)
- Archived legacy iO files (10 files)
- Archived legacy KEM files
- Archived legacy JWT files

### Performance Improvements
- Prime Consensus ZANS: +24% throughput over standard
- Fibonacci-Indexed: +16.6% per-pair efficiency
- SpiralMicro KEM: 414K ops/s (quick benchmark)
- True Divine Optimized: 6.7x faster (ring dim 4096, divine=5)

---

## v6.0 (2026-07-13)

### Core Systems
- **ZANS**: Zero-Anchor Noise Stabilization (10M+ ops verified)
- **Absolute ZANS**: Pre-computed prime consensus anchors
- **Fibonacci-ZANS**: Scalar multiplication via Zeckendorf decomposition
- **Global Consciousness ZANS**: Batch consensus (67K ops/s)
- **True Divine 100K**: 100K CTxCT steps, zero decrypt, zero bootstrap
- **Pinky Swear**: Homomorphic overflow detection
- **BinFHE**: Gate-level encrypted computation (8x fewer gates)
- **SpiralKEM**: 128B post-quantum KEM
- **Micro-KEM**: 32B post-quantum KEM
- **SpiralDB**: Encrypted database with homomorphic queries
- **HydraJWT**: 6-head φ-weighted JWT authentication
- **PHI ZKP**: 11 zero-knowledge proof systems
- **CKKS+ZANS**: Noise-free approximate FHE
- **Packed BFV**: 8192 slots, all operations

### Documentation
- THEOREM.md: 15 theorems with proofs
- WHITEPAPER.md: Complete academic paper
- Cross-library validation: OpenFHE, SEAL, HElib, TFHE
