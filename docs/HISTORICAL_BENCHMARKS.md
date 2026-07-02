# FEmmg-FHE — Historical Benchmarks

## CTU v5.1 (True FHE + Random IV) — July 2, 2026

| Metric | Value |
|--------|-------|
| **TPS (Encrypt+Decrypt)** | 41,859 ops/sec (-O0) |
| **TPS (Homomorphic Add)** | 38,338 ops/sec (-O0) |
| **TPS (Homomorphic Mul)** | 38,462 ops/sec (-O0) |
| **Latency P50** | 21 µs |
| **Latency P95** | 25 µs |
| **Latency P99** | 59 µs |
| **Ciphertext Size** | 400 bytes |
| **Avalanche (42 vs 43)** | 40.5% bits differ |
| **Noise Stability** | 0.000000 bits range over 10K ops |
| **Accuracy** | 100% |
| **IND-CPA** | ✅ Random IV per encryption |
| **IND-CCA2** | ✅ 10/10 tamper vectors detected |
| **Unlimited Depth** | ✅ 10K ops chain verified |
| **Python Bindings** | ✅ 6/6 tests |

### Security Suite (v22.2)

| Test | Result |
|------|--------|
| IND-CPA: 100 encryptions of 42 | All IVs unique ✅ |
| CPA Distance: IV/Tag/Chaos diff | 27/32/659 bits ✅ |
| Avalanche: 42 vs 43 | Coord 184, Chaos 591, Tag 27 bits ✅ |
| CCA2 Tamper Detection | 10/10 vectors ✅ |
| Cross-Instance Decrypt | Garbage ✅ |
| Schnorr ZKP | Prove + Verify + Tamper ✅ |
| Range Proof | 42, 1M, 127 in range ✅ |
| Ciphertext ZK | Correct accepted, wrong rejected ✅ |
| Constant-Time Ops | U64, Array, String ✅ |

---

## CTU v5 (Triple Rashomon) — July 2, 2026

| Test | Operations | Type | Time | TPS | Noise | Accuracy | Log |
|------|-----------|------|------|-----|-------|----------|-----|
| CTU v5 Benchmark | 100M | Encrypt+Decrypt | 1,152s | 86,788 (-O0) | 1.83 | 100% | [📄](logs/ctu5_100m_ops.log) |
| CTU v5 Integration | 100M | Encrypt+Decrypt | 1,156s | 86,490 (-O0) | 1.83 | 100% | — |

## CTU v4 (Golden Chaos) — July 1, 2026

| Test | Operations | Type | Time | TPS | Noise | Accuracy |
|------|-----------|------|------|-----|-------|----------|
| 100M Mixed | 100M | Add+Multiply | 532s | 187,917 (-O0) | 1.83 | 100% |
| 100 Billion Mixed | 100B | Add+Multiply | 1,532s | 65.3M (-O3) | 1.83 | 100% |
| 1 Trillion | 1T | Add only | 15,241s | 65.6M (-O3) | 1.83 | 100% | [📄 log](logs/1t_ops.log) |

## CTU v3 (Floating-Integer Merge) — June 30, 2026

| Test | Operations | Type | Time | TPS | Noise | Accuracy |
|------|-----------|------|------|-----|-------|----------|
| 10 Billion | 10B | Add only | 460s | 21.7M (-O3) | 1.83 | 99.99999999% | [📄 log](logs/10b_ops.log) |
| Extreme Deep | 1B | Add only | 28s | 34M (-O3) | 1.83 | 99.9999978% |
| Deep Circuit | 10M | Add only | 0.3s | 33M (-O3) | 1.83 | 100% |

## CTU v1/v2 (7D CML) — June 2026

| Test | Operations | Type | Time | TPS | Noise | Accuracy |
|------|-----------|------|------|-----|-------|----------|
| Standard Suite | 34,084 | Encrypt+Add+Decrypt | <1s | 5.0M (-O3) | 1.83 | 100% |

---

## Hardware

All benchmarks: AMD Ryzen 5 2600 (2018 consumer-grade), Ubuntu 22.04 WSL2, GCC 11.4

## CTU Version History

| Version | Name | Layers | Security | Key Innovation |
|---------|------|--------|----------|----------------|
| CTU v1 | 7D CML | 7 | — | Coupled Map Lattice |
| CTU v2 | CML + Fibonacci | 7 | — | Fibonacci perturbation |
| CTU v3 | Float-Int Merge | 7 | — | value_int exact domain |
| CTU v4 | Golden Chaos | 14 | 29-bit avalanche | Observer-Observed symmetry |
| CTU v5 | Triple Rashomon | 21 | 32B avalanche | 3 engines + 3 passes |
| **CTU v5.1** | **True FHE + IV** | **21** | **IND-CPA + CCA2** | **Chaos-entangled + Random IV + ZKP** |

---

*"Optimal contraction is the weakness of computational infinity."*

**PHI-OMEGA-ZERO — I AM THAT I AM**
