# Benchmarks

All benchmarks on consumer hardware (AMD Ryzen 5 2600, 16GB RAM) with OpenFHE CKKS.

---

## Summary

| Test | RingDim | Max Mults | Max Boots | Final φ-error | ψ-noise range |
|------|---------|-----------|-----------|---------------|---------------|
| Boot-free V2 | 4096 | 201 | 0 | 6.3×10⁻¹³ | 10⁻¹⁴–10⁻¹⁵ |
| Unlimited | 8192 | 738 | 3 | 2.9×10⁻⁸ | 10⁻¹²–10⁻¹³ |
| Sprint (Run 1) | 8192 | 1887 | 29 | 1.1×10⁻⁷ | 10⁻¹²–10⁻¹³ |
| Sprint (Run 2) | 8192 | 6300 | 100 | 1.98×10⁻⁷ | 10⁻¹¹–10⁻¹² |
| Complete | 4096 | 405 | 10 | — | 2.6×10⁻⁸ |

---

## φ-Error Growth Rate

| Test | Error Rate (per mult) | R² |
|------|----------------------|-----|
| Boot-free V2 | ~5×10⁻¹⁶ | — |
| Sprint Run 1 | ~5.9×10⁻¹¹ | >0.999 |
| Sprint Run 2 | ~3.2×10⁻¹¹ | >0.999 |
| Unlimited | ~3.8×10⁻¹¹ | >0.999 |

**Consistent linear growth across all tests.**

---

## Projected Depth to Error Thresholds

| Error Threshold | Projected Multiplications |
|-----------------|---------------------------|
| 0.01% (10⁻⁴) | ~3 million |
| 0.1% (10⁻³) | ~30 million |
| 1% (10⁻²) | ~300 million |

---

## Bootstrap Recovery

Post-bootstrap ψ-noise spike: ~10⁻¹⁴
Recovery time: 2 clean cycles (6 multiplications)
Post-recovery ψ-noise: ~10⁻¹²–10⁻¹³ (same as pre-bootstrap baseline)

**No accumulation across bootstrap boundaries.**

---

## Hardware

| Component | Spec |
|-----------|------|
| CPU | AMD Ryzen 5 2600 (6-core, 3.40 GHz) |
| RAM | 16 GB |
| GPU | Radeon RX 580 (8 GB) |
| OS | Linux x64 |

---

## Security Levels

| RingDim | Security | Slots | Tested |
|---------|----------|-------|--------|
| 4096 | TOY | 2048 | ✓ |
| 8192 | TOY | 4096 | ✓ |
| 32768 | 128-bit classic | 16384 | Partial |
| 65536 | 256-bit | 32768 | Not tested |

*RingDim=32768 requires >16GB RAM for full bootstrap testing.*
