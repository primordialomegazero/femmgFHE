# Contributing to Spiral FHE+iO

Thank you for your interest in contributing. This project began as a solo effort and grew into a complete FHE+iO system. Contributions are welcome, provided they maintain the mathematical and architectural integrity of the system.

---

## Current State (Honest Assessment)

| Component | Status | Test Evidence |
|-----------|--------|---------------|
| FHE Unlimited Bootstrap | Production | 10,000 cycles, 9.51 c/s, 0.01% Cassini warnings |
| PFE TFHE Universal Circuit | Production | 1M gates, 10.18s, XOR 4/4 |
| DualGate Bridge | Core proven | CKKS→TFHE conversion PASS |
| TEE Transport | Engineering | Serialization works (44.8MB), socket integration pending |
| Hardware TEE (SGX/TrustZone) | Not implemented | Requires hardware + SDK |

---

## Quick Start

```bash
git clone https://github.com/primordialomegazero/femmgFHE.git
cd femmgFHE
```

**Prerequisites:**
- Linux (WSL2 works)
- 16GB RAM recommended
- GCC 11+ or Clang 14+
- OpenFHE v1.5.1 (stable release, not development branch)
- GMP, NTL

**Build:**
```bash
# Manual build (simplest)
g++ -std=c++17 -O3 -o test_fhe tests/test_production.cpp \
    -I. -Iopenfhe-development/src/pke/include \
    -Iopenfhe-development/src/core/include \
    -Iopenfhe-development/src/binfhe/include \
    -Lopenfhe-development/build/lib \
    -lOPENFHEpke -lOPENFHEcore -lOPENFHEbinfhe -lntl -lgmp -lm
```

**Run quick tests:**
```bash
./test_fhe            # FHE 10-cycle bootstrap
./test_io_tfhe        # iO 4-gate XOR
./test_bridge_simple  # DualGate bridge
```

---

## What You Can Contribute

| Area | Priority | Difficulty | Notes |
|------|----------|------------|-------|
| Bug Reports | High | Easy | Report with reproducible test case |
| Documentation | High | Easy | Tutorials, API docs, examples |
| Language Bindings | Medium | Medium | Python, C already exist |
| FHE Applications | Medium | Hard | AES, SHA, ML, DB on top of FHE |
| Test Coverage | Medium | Medium | More edge cases, property tests |
| Performance Optimization | Medium | Hard | GPU/FPGA, batch processing |
| Hardware TEE | Low | Extreme | SGX/TrustZone integration |
| Core φ-Math | Low | Extreme | Requires deep understanding |

---

## Development Setup

**Requirements:**
- Linux (WSL2 works)
- 8GB minimum RAM, 16GB recommended
- GCC 11+ or Clang 14+
- OpenFHE v1.5.1 stable (NOT development branch — has gate mapping bugs)

**Known OpenFHE Issues:**
- Development branch: TFHE gates swapped (NAND→NOR, AND→OR)
- Fix: Use stable v1.5.1 release
- TOY parameters: too noisy for TFHE gates
- Fix: Use STD128

---

## Key Files

| File | Purpose |
|------|---------|
| `src/fhe/spiral_fhe_io_final.h` | FHE bootstrap (163 lines) |
| `src/fhe/decrypt_layer.h` | SK isolation + GF-N |
| `src/io/spiral_io_tfhe.h` | iO TFHE universal circuit |
| `src/bridge/dual_gate_bridge_fixed.h` | FHE↔iO bridge |
| `src/bridge/tee_dual_gate_bridge.h` | TEE transport |
| `src/core/constants.h` | φ·ψ = -1 constants |
| `src/config/gf_n_encryption.h` | GF-N encryption engine |
| `docs/FORMAL_PROOF_FINAL.md` | Formal security proof |

---

## Pull Request Process

1. Fork the repository
2. Create feature branch: `git checkout -b feat/my-feature`
3. Write code with documentation (WHY, not just WHAT)
4. Test: all existing tests must pass
5. Commit with clear message
6. Push and open PR

**PR Requirements:**
- [ ] All existing tests pass
- [ ] New code documented (mathematical/architectural reason)
- [ ] No hardcoded magic numbers — use named constants
- [ ] φ·ψ = -1 invariants preserved (if modifying core)
- [ ] License header in all new files
- [ ] Test evidence included (what you ran, what it measured)

---

## Code Style

**Philosophy:**
- Document WHY, not just WHAT
- Every struct/function needs header comment explaining the mathematical reason
- φ/ψ duality clear in naming
- Compile-time verification where possible
- N-configurable — no hardcoded constants

**Example:**
```cpp
// DUALGATE PROJECTION — Map (a,b) pair to φ-basis
// φ_val = a·φ + b·ψ, ψ_val = a·ψ + b·φ
// Product invariant: φ_val · ψ_val = -a² + 3ab - b²
// Used by: FHE↔iO bridge conversion
struct DualGateFixed {
    double phi_val, psi_val;
    DualGateFixed(double a, double b) {
        phi_val = a * PHI + b * PSI;
        psi_val = a * PSI + b * PHI;
    }
};
```

---

## Mathematical Integrity

**What is proven (theorems):**
- `φ·ψ = -1` — algebraic identity
- `φ²+ψ² = 3` — algebraic identity
- `FGG(v, d) = |v|` for d ≥ 1 — structural erasure
- `DualGate projection = -a² + 3ab - b²` — invariant

**What is standard assumptions (not proven, widely accepted):**
- CKKS IND-CPA security (Ring-LWE)
- TFHE scheme security
- GF-N key secrecy (symmetric cipher)

**What is NOT claimed:**
- We do NOT claim P=NP
- We do NOT claim Bitcoin private key recovery
- We do NOT claim quantum supremacy
- We do NOT claim breaking any existing cryptographic scheme

**Any contribution modifying core math MUST:**
1. Preserve `φ·ψ = -1` invariant
2. Not introduce new hardness assumptions
3. Document which theorems/assumptions are used
4. Provide test evidence

---

## Communication

- **Issues:** GitHub Issues for bugs and feature requests
- **Email:** devilswithin13@gmail.com for private inquiries
- **Subject line:** Use "Spiral FHE+iO — [Topic]"

---

## Recognition

All contributors listed in repository. Significant contributions may qualify for co-authorship on academic papers.

---

*"The security is structural, not computational. φ·ψ = -1 is a theorem, not a conjecture."*

*— Dan Joseph M. Fernandez*
