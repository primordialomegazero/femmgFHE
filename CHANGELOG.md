# FEmmg-FHE Changelog

All notable changes to FEmmg-FHE will be documented in this file.

---

## [v17.2.0] — FORTRESS Upgrade C: Unified Φ-Stack Integration (2026-06-30)

### Added
- **Unified Φ-Stack Integration**: Native `unified_pipeline` endpoint
  - Φ-SIG Keyless Authentication (placeholder)
  - Spiralkem-FHE Post-Quantum KEM Handshake (placeholder)
  - pozDF-FHE Encrypted Computation (full 7D Banach)
  - SpiralDB Encrypted Storage (Double Mirror)
  - SchupyFHE Earth Frequency Gate (7.83 Hz ± 0.2 Hz)
- `fhe_encrypt` endpoint — server-side 7D Banach encryption
- `phi_stack.h` — Unified integration header
- Health endpoint shows `unified_phi_stack: true` + session count

### Changed
- `fhe_add`/`fhe_multiply` now properly set `expanded_dim0` for backward compatibility
- Health endpoint: version → 17.2.0, engine → FORTRESS v17.2

### Performance
- TPS: **1.11M** (True 7D Banach FHE)

---

## [v17.2.0] — FORTRESS Upgrade B: Full 7D CML Server API (2026-06-30)

### Added
- `fhe_encrypt` endpoint: Server-side encryption using full `godcode::NDimBanachEngine`
- Health endpoint shows `tps_boost_precomputed: true`

### Fixed
- Backward compatible `fhe_add`/`fhe_multiply` with proper `expanded_dim0`

---

## [v17.2.0] — FORTRESS Upgrade A: TPS Boost (2026-06-30)

### Added
- **Pre-computed perturbation table** `[DIMS][DEPTH][PARTIES]`
  - Eliminated ALL `sin()` calls at runtime (49 per op → 0)
  - Built once at engine construction

### Performance
- TPS: **0.26M → 1.11M** (4.2× improvement)
  - Removed 49 `sin()` calls per operation
  - All encrypt/decrypt/recontract use fast array lookup

---

## [v17.1.0] — FORTRESS: Cached Expanded Dim0 (2026-06-30)

### Added
- `NDimCiphertext.expanded_dim0` — caches pre-contraction value for fast homomorphic ops
- `NDimBanachEngine.recontract_dim0()` — one-way fast re-contraction
- NPM Client v17.1.0: 7D Sine-Coupled Map Lattice IND-CPA
  - Replaced 1D logistic map with 7D Sine-CML
  - `crypto.randomBytes(4)` per encryption (32-bit true entropy)

### Performance
- TPS: **0.24M → 0.26M** (cached expand path)

---

## [v17.0.0] — FORTRESS: Path X — Full 7D Banach Integration (2026-06-29)

### Added
- **Path A: Complete Mathematical Reversal**
  - Decryption reverses all perturbation in exact reverse order (DEPTH-1 → 0)
  - `godcode::NDimBanachEngine` with 7D Banach contraction
- **Path X: Full Integration**
  - `femmg_fhe.h`: Expand/Contract add/multiply
  - `femmg_server.cpp`: FEmmgFHE engine replaces bare LyapFHE
  - `fractal_fhe.h`: NDimCiphertext throughout
  - `test_suite.cpp`: 34,084 tests
- Cross-party verification: 91/91 pairs across 14 parties
- Zero warnings (-Wall -Wextra -Werror)

### Fixed
- **Critical**: Decryption now properly reverses perturbation (was missing reversal)
- verify_contraction: exempts dim 0 (data carrier), checks dims 1-6 only

### Performance
- TPS: **0.24M** (True 7D Banach, down from 14M fake)

---

## [v16.2.0] — IACR Submission (2026-06-28)

### Added
- Three-Layer Noise Stabilization: Banach + Lyapunov + Phi-Zeta
- Two-Phase Encryption: Client-side IND-CPA + Server-side blind compute
- CTU Assumption formalized
- 6 formal theorems with algebraic proofs
- 30/30 Dark Abyss Gauntlet, 24/25 Alien Wolves Attack Suite
- φ-modulation in Riemann zeta zeros (supplementary)

---

## [v10.1.0] and earlier

- Initial Lyapunov-Coupled FHE
- Blind Multiplication formula: `(e1·e2 - λ(e1+e2) + λ²)/φ + λ`
- Banach Fixed Point Theorem for noise stabilization
- φ-contraction: `T(x) = x·φ⁻¹ + N₀·(1-φ⁻¹)`
- Golden Ratio self-reference: `φ = 1 + 1/φ`
- CORE Security: Multi-layer attack immunity
- Docker + NPM deployment

---

## Legend

| Icon | Meaning |
|------|---------|
| 🔥 | Major breakthrough |
| ⚡ | Performance improvement |
| 🛡️ | Security enhancement |
| 🐛 | Bug fix |
| 📦 | New feature |
| 📝 | Documentation |

---

> *"Golden ratio is simply the weakness of infinity."* — Dan Fernandez

> *ΦΩ0 — I AM THAT I AM*


