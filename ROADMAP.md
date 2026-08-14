# Roadmap — Remaining Work

**Version 2.0**

---

## Purpose

This document lists the engineering and research work needed to move this prototype toward a validated scheme.

The current implementation is a small-scale research prototype (N=1024, Q=2^29). It is not production software.

---

## Current State

| Component | Status | Scale |
|-----------|--------|-------|
| FHE core (encrypt/decrypt/NAND) | Tested | N=1024, Q=2^29 |
| Bootstrapping | Tested | 20 levels |
| Circuit obfuscation | Tested | 4-input XOR 16/16 |
| Golden Angle PRNG | Tested | 1M/1M unique |
| Lucas commitment | Tested | 0/100K collisions |
| Equidistributed noise | Tested | Balance 0.0002 |
| Attack resistance | Tested | 8 classes blocked |
| Documentation | Written | 10+ files |

---

## Priority Work

### High Priority

| Task | Why | Effort |
|------|-----|--------|
| Peer review | Validation by independent researchers | Low effort (submit) |
| Larger Q (2^60+) | Current Q=2^29 too small for real security | Low |
| NTT multiplication | Current O(N²) is slow; NTT gives O(N log N) | Medium |
| Formal security analysis | Current claims rely on RLWE informally | High |

### Medium Priority

| Task | Why |
|------|-----|
| CMake build | Easier compilation |
| Test framework | Organized test runner |
| Key switching | Needed for real FHE |
| Relinearization | Reduce ciphertext size |

### Low Priority

| Task | Why |
|------|-----|
| GPU acceleration | Performance only |
| Formal verification (Coq) | Very high effort |
| MPC integration | Feature extension |

---

## Phases

### Phase 1: Validation (Weeks 1-4)

- [ ] Submit for peer review
- [ ] Independent security analysis
- [ ] Bug bounty setup
- [ ] Community feedback

### Phase 2: Engineering (Weeks 5-12)

- [ ] NTT polynomial multiplication
- [ ] Key switching
- [ ] Relinearization
- [ ] Serialization
- [ ] CMake build
- [ ] CI/CD

### Phase 3: Security Hardening (Weeks 13-20)

- [ ] Larger parameters (Q=2^60+)
- [ ] LWE estimator integration
- [ ] Formal verification
- [ ] Side-channel analysis

### Phase 4: Extensions (Ongoing)

- [ ] Matrix Branching Programs
- [ ] Functional encryption
- [ ] MPC
- [ ] ZK proofs

---

## What This Roadmap Does NOT Include

- Claims of production readiness
- Claims of security beyond RLWE
- Claims of solving open problems

---

## Contact

**Email:** devilswithin13@gmail.com

---

*This roadmap is honest about the current state. The prototype is small-scale. Significant work remains before this can be considered a validated scheme.*
