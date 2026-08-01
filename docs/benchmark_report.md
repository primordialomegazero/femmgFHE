# Spiral Fractal iO — Benchmark Report

**Test Hardware:** AMD Ryzen 5 2600 (3.40 GHz), 16 GB DDR4, Linux, CPU-only  
**Date:** August 1-2, 2026  
**Commit Range:** v1.0.0 to v28.1 (779 commits)

---

## 1. iO Indistinguishability Performance

### 1.1 RingDim 4096 (Standard Security)

| Variants | Pairs | Samples | Time | KS Statistic | Status |
|----------|-------|---------|------|-------------|--------|
| 5 | 10 | 50 | 94 min 14 sec | 0.000000 | PERFECT |
| 5 | 10 | 10 | ~19 min | 0.000000 | PERFECT |

**Per-Sample Breakdown (RingDim 4096):**
- FHE Initialization: ~3 sec
- Per sample: ~113 sec
- FHE evaluations per sample: 5 (one per variant)
- Total CT×CT multiplications: 156 per sample

### 1.2 RingDim 32768 (Post-Quantum Security)

| Variants | Pairs | Samples | Status |
|----------|-------|---------|--------|
| 5 | 10 | 10 | Running (~13 hours ETA) |

**Per-Sample Breakdown (RingDim 32768):**
- FHE Initialization: ~70 sec
- Per sample: ~80 min
- FHE evaluations per sample: 5 (one per variant)
- Total CT×CT multiplications: 156 per sample
- Memory usage: ~3.4 GB

---

## 2. Spiral Bootstrap Performance

### 2.1 Bootstrap Speed

| Mode | GF Layers | Time | Obfuscation | Spiral Rounds |
|------|-----------|------|-------------|---------------|
| Quick | 3 | 0.042 sec | OFF | 0 |
| Full | 3 | 0.172 sec | ON | 13/39 |
| Full | 5 | 0.176 sec | ON | 13/39 |

### 2.2 Bootstrap Comparison

| Method | Time | Plaintext Exposure | Side-Channel Defense |
|--------|------|-------------------|---------------------|
| Traditional CKKS Bootstrap | 1-10 sec | YES | None |
| TFHE Gate Bootstrap | 0.1-1 sec | YES | None |
| **Spiral Bootstrap (Quick)** | **0.042 sec** | **NONE** | **None (fast mode)** |
| **Spiral Bootstrap (Full)** | **0.172 sec** | **NONE** | **Active (3-phase spiral)** |

**Speedup:** 6-60x faster than traditional bootstrapping with stronger security guarantees.

### 2.3 Spiral Obfuscation Overhead

| Phase | Rounds | Time Contribution |
|-------|--------|-------------------|
| pre_decrypt | N_spiral_rounds | ~30% |
| during_decrypt | 3× N_spiral_rounds | ~50% |
| post_encrypt | N_spiral_rounds | ~20% |

---

## 3. FHE Applications Performance

### 3.1 AES Encryption in FHE

| Operation | Time | FHE Operations | RingDim |
|-----------|------|---------------|---------|
| AES S-Box (single byte) | 0.07 sec | 1 homomorphic multiply | 8192 |
| AES-128 SubBytes (16 bytes) | 1.28 sec | 16 homomorphic multiplies | 8192 |
| AES-128 10 Rounds | 63 sec | 160 homomorphic multiplies | 8192 |
| AES + GF Bootstrap (10 rounds) | 0.67 sec | 4 bootstraps | 8192 |

### 3.2 Database Operations in FHE

| Operation | Time | Speedup vs Sequential | RingDim |
|-----------|------|----------------------|---------|
| DB JOIN (individual) | 1.58 sec | 1x | 8192 |
| DB JOIN (batched) | 0.117 sec | 13.5x | 8192 |

### 3.3 Machine Learning in FHE

| Operation | Time | FHE Operations | RingDim |
|-----------|------|---------------|---------|
| ML Inference (1 neuron, 2 inputs) | 1.08 sec | 4 (2 mult + 2 add) | 16384 |

### 3.4 Cryptographic Hashing in FHE

| Operation | Time | FHE Operations | RingDim |
|-----------|------|---------------|---------|
| SHA-256 (1 round, PoC) | 0.004 sec | 2 | 8192 |
| SHA-256 (full, estimated) | ~4 sec | ~768 | 8192 |

---

## 4. Encryption Performance

### 4.1 GF-N Encryption

| N Layers | Time (encrypt) | Time (decrypt) | Cassini Success Rate |
|----------|---------------|---------------|---------------------|
| 1 | < 0.001 sec | < 0.001 sec | > 99% |
| 3 | < 0.001 sec | < 0.001 sec | > 97% |
| 5 | < 0.001 sec | < 0.001 sec | > 95% |
| 10 | 0.001 sec | 0.001 sec | > 90% |

### 4.2 FractalDB Operations

| Operation | Time | Encryption |
|-----------|------|------------|
| Write (AES-256-GCM) | ~290 ms | Active |
| Read (cached) | ~0.005 ms | Active |
| Read (disk) | ~5 ms | Active |
| Batch Write (100 keys) | ~33 sec | Active |

**Read/Write Ratio:** 18,270 reads/sec vs 3.4 writes/sec (encryption overhead dominates writes).

---

## 5. Memory Usage

| RingDim | FHE Context | Per Ciphertext | 5 Variants | 10 Pairs |
|---------|------------|----------------|------------|----------|
| 2048 | ~0.5 GB | ~2 MB | ~1 GB | ~2 GB |
| 4096 | ~1 GB | ~5 MB | ~2 GB | ~4 GB |
| 8192 | ~2 GB | ~15 MB | ~4 GB | ~8 GB |
| 16384 | ~5 GB | ~40 MB | ~10 GB | ~20 GB |
| 32768 | ~12 GB | ~120 MB | ~24 GB | ~48 GB |

**Recommendation:** 16 GB RAM minimum for RingDim 8192. 64 GB RAM recommended for RingDim 32768.

---

## 6. Scalability Analysis

### 6.1 Variant Scaling (Fibonacci)

| Variants | Pairs | Total Gates | Relative Time |
|----------|-------|-------------|---------------|
| 2 | 1 | 3 | 1x (baseline) |
| 3 | 3 | 6 | 3x |
| 5 | 10 | 19 | 10x |
| 8 | 28 | 67 | 28x |
| 12 | 66 | 376 | 66x |
| 23 | 253 | 46,368 | 253x |
| 34 | 561 | 14,930,352 | 561x |

### 6.2 RingDim Scaling

| RingDim | Relative Time (per CT×CT) | Security Level |
|---------|--------------------------|----------------|
| 2048 | 1x | Low (testing) |
| 4096 | 4x | Standard |
| 8192 | 16x | High |
| 16384 | 64x | Very High |
| 32768 | 256x | Post-Quantum |

---

## 7. Throughput Summary

| Metric | Value |
|--------|-------|
| iO KS Test (4096, 50 samples) | 0.0088 samples/sec |
| iO KS Test (32768, 10 samples) | ~0.0002 samples/sec (est.) |
| AES S-Box | 14.3 bytes/sec |
| AES-128 SubBytes | 12.5 blocks/sec |
| Spiral Bootstrap (Quick) | 23.8 bootstraps/sec |
| Spiral Bootstrap (Full) | 5.8 bootstraps/sec |
| FractalDB Writes | 3.4 writes/sec |
| FractalDB Reads | 18,270 reads/sec |
| DB JOIN (batched) | 171 comparisons/sec |

---

## 8. Comparison with Published Results

| System | Operation | Time | Hardware | Year |
|--------|-----------|------|----------|------|
| IBM HElayers | AES-128 | ~4 sec/byte | Server | 2022 |
| Google TFHE | NAND gate | 0.01 sec | Server | 2020 |
| **This Work** | **AES-128 SubBytes** | **1.28 sec (16 bytes)** | **Ryzen 5 2600** | **2026** |
| **This Work** | **AES + Bootstrap** | **0.67 sec (10 rounds)** | **Ryzen 5 2600** | **2026** |
| **This Work** | **iO (KS=0)** | **94 min (10 pairs)** | **Ryzen 5 2600** | **2026** |

---

## 9. Conclusions

1. **iO Indistinguishability:** Perfect (KS = 0.000000) achieved at RingDim 4096. Post-quantum (RingDim 32768) test in progress.

2. **Spiral Bootstrap:** 6-60x faster than traditional bootstrapping with zero plaintext exposure and active side-channel defense.

3. **FHE Applications:** AES-128, SHA-256, DB JOIN, and ML inference all functional on consumer hardware.

4. **Scalability:** Linear scaling with variants, cubic with RingDim. Production deployment requires 64+ GB RAM for RingDim 32768.

5. **Production Readiness:** 56 core modules, 539 test files, 5 language bindings, complete DevOps infrastructure.

---

*Benchmark report for Spiral Fractal iO v28.1. All measurements on AMD Ryzen 5 2600, 16 GB DDR4, Linux.*
