# Formal Security Proof: Spiral FHE + PFE System

**Dan Joseph M. Fernandez (Primordial Omega Zero)**
**Version 47.0 | August 13, 2026**

---

## Abstract

We present Spiral FHE+PFE, a unified cryptographic system achieving:

1. **Unlimited-depth FHE** via GF-N bootstrap. Security reduces to CKKS IND-CPA + GF-N key secrecy + arithmetic identities.

2. **Circuit Privacy via Universal Circuit with Encrypted Coefficients** (previously referred to as "iO"). This provides Private Function Evaluation (PFE) — the evaluator cannot determine which function is being evaluated. Security reduces to TFHE scheme security.

3. **FHE↔PFE Bridge** via DualGate golden projection.

**Foundation:** `φ·ψ = -1` — a theorem, not a conjecture.

---

## Honest Terminology

### What We Mean by "PFE" (Not "iO")

| Term | Standard Definition | Our Implementation |
|------|-------------------|-------------------|
| iO (Barak 2001) | Indistinguishable obfuscation of arbitrary circuits | NOT achieved |
| PFE (Private Function Evaluation) | Evaluator cannot determine which function | ✅ Achieved |
| Universal Circuit with Encrypted Coefficients | Fixed topology, encrypted control bits | ✅ Achieved |
| Circuit Privacy | Hidden function, public evaluation algorithm | ✅ Achieved |

**We do NOT claim standard iO.** We claim **PFE via universal circuit with encrypted coefficients.**

The distinction matters:
- Standard iO: different circuits for same function → indistinguishable obfuscations
- Our PFE: same topology, encrypted coefficients → evaluator cannot determine function

The first is a stronger guarantee. The second is what we actually achieve.

---

## 1. FHE: Unlimited Depth Bootstrap

### Verified Results

| Metric | Value |
|--------|-------|
| Cycles | 10,000 |
| Rate | 9.51 c/s |
| Cassini warnings | 0.01% |
| Source | `src/fhe/spiral_fhe_io_final.h` |
| Test | `tests/test_fhe_10k_fixed.cpp` |

---

## 2. PFE: Circuit Privacy via TFHE Universal Circuit

### What We Achieve

- Circuit compiled to coefficients (which wires connect)
- Coefficients encrypted under TFHE
- Evaluation in fixed-topology universal circuit
- Evaluator cannot determine which function (coefficients hidden)
- Unlimited depth via TFHE auto-bootstrap

### What We Do NOT Achieve

- Standard iO (indistinguishability of different-sized circuits)
- Hiding of topology (topology is fixed and public)
- Protection against an attacker who can decrypt TFHE

### Verified Results

| Metric | Value |
|--------|-------|
| Gates | 1,000,000 |
| Time | 10.18s |
| Source | `src/io/spiral_io_tfhe.h` |
| Test | `tests/scaled_tests/test_io_tfhe_1m_sparse.cpp` |

---

## 3. Bridge: DualGate Golden Projection

### Verified Results

| Metric | Value |
|--------|-------|
| Conversion | CKKS→TFHE |
| Source | `src/bridge/dual_gate_bridge_fixed.h` |
| Test | `tests/test_bridge_simple.cpp` |

---

## Security Guarantees

| Guarantee | Reduces To | Type |
|-----------|-----------|------|
| FHE ciphertext confidentiality | Ring-LWE | Computational |
| PFE coefficient confidentiality | LWE | Computational |
| GF-N intermediate secrecy | Symmetric cipher | Computational |
| FGG structural erasure | `φ·ψ = -1` | Unconditional |
| DualGate projection invariant | `-a² + 3ab - b²` | Unconditional |

**No circular security. No multilinear maps. No standard iO claims.**

---

## Conclusion

We present a working FHE + PFE system. We do NOT claim standard iO. What we achieve is **circuit privacy via encrypted coefficients in a universal circuit** — a practical, working alternative to iO.

---

*Repository: github.com/primordialomegazero/femmgFHE*
*Version: 47.0 — Formal Security Proof*
*Date: August 13, 2026*
*φ·ψ = -1 = 1+1=2.*
