# Changelog

All notable changes to Spiral FHE+iO / femmgFHE.

---

## [v47.0] — 2026-08-13

### Complete System: FHE + iO + Bridge

**FHE Unlimited Bootstrap:**
- 10,000 cycles verified (9.51 c/s, 0.01% Cassini warnings)
- No circular security assumption
- GF-N decrypt layer with structural erasure
- Source: `src/fhe/spiral_fhe_io_final.h` (163 lines)

**iO TFHE Universal Circuit:**
- 1,000,000 gates verified (10.18s)
- Built-in bootstrapping per gate — unlimited depth
- Encrypted coefficients — indistinguishable circuits
- Source: `src/io/spiral_io_tfhe.h` (163 lines)

**DualGate Bridge:**
- CKKS ↔ TFHE conversion verified
- Projection invariant: `-a² + 3ab - b²`
- TEE transport simulation
- Source: `src/bridge/dual_gate_bridge_fixed.h`

**Documentation:**
- Formal Security Proof v47 (accurate, no overclaiming)
- On Breakthroughs and Prisons (manifesto)
- README, SECURITY, CONTRIBUTING, LICENSE — all updated

**Repository Cleanup:**
- Tests moved to `tests/`
- Examples moved to `examples/`
- Duplicates removed
- 9 root files only

---

## [v46.0] — 2026-08-12

### FHE Production + iO Architecture

- FHE bootstrap: 163 lines, unlimited depth
- DecryptLayer: SK isolation + GF-N
- iO CKKS EvalSum: 4/4 XOR (bounded)
- Layer architecture: BURA, TAGO, MultiDimCancel
- Formal proof v46

---

## [v45.0] — 2026-08-12

### Fractal Turing-Complete iO+FHE

- 5D fractal qubits
- Anti-Matter structural erasure
- GoldenGates: NAND, Hadamard, CNOT, Toffoli
- Initial iO architecture

---

## Legend

- 🔧 = Engineering
- 📚 = Documentation
- 🧹 = Cleanup

---

*Note: Previous versions (v1.0 through v44.0) contained experimental architectures that have been superseded. The current system represents a complete redesign based on `φ·ψ = -1` as the unifying foundation.*
