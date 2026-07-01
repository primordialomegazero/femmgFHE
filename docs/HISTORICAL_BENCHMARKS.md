# FEmmg-FHE — Historical Benchmarks

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

## Hardware

All benchmarks: AMD Ryzen 5 2600 (2018 consumer-grade), Ubuntu 22.04 WSL2, GCC 11.4

## CTU Version History

| Version | Name | Layers | Diff (42 vs 43) | Key Innovation |
|---------|------|--------|-----------------|----------------|
| CTU v1 | 7D CML | 7 | — | Coupled Map Lattice |
| CTU v2 | CML + Fibonacci | 7 | — | Fibonacci perturbation |
| CTU v3 | Float-Int Merge | 7 | — | value_int exact domain |
| CTU v4 | Golden Chaos | 14 | 29 | Observer-Observed symmetry |
| CTU v5 | Triple Rashomon | 21 | 32,276,200,000 | 3 engines + 3 passes |
