# Security Policy

## Supported Versions

| Version | Supported | Notes |
|---------|-----------|-------|
| v47.x | ✅ Active | Current stable: FHE + iO + Bridge |
| v46.x | ✅ Active | FHE + iO (pre-bridge) |
| v45.x | ⚠️ Maintenance | Fractal Turing-Complete (legacy) |
| < v45 | ❌ End of life | Historical architectures |

---

## Reporting a Vulnerability

**DO NOT open a public issue.**

Email: devilswithin13@gmail.com

Include:
- Affected version
- Steps to reproduce
- Impact assessment
- Optional: Suggested fix

### Response Time

| Severity | Response |
|----------|----------|
| Critical (plaintext exposure, scheme break) | 24 hours |
| High (security bypass) | 48 hours |
| Medium (performance, memory) | 7 days |
| Low (documentation) | 14 days |

---

## Security Model

Security is **layered**: computational (standard assumptions) + structural (arithmetic identities).

### What is Proven (Theorems)

| Property | Mechanism | Source |
|----------|-----------|--------|
| `φ·ψ = -1` | Golden identity | `src/core/constants.h` |
| `FGG(v,d) = \|v\|` for d ≥ 1 | Structural erasure | `src/fhe/spiral_fhe_io_final.h` |
| DualGate projection invariant | `-a² + 3ab - b²` | `src/bridge/dual_gate_bridge_fixed.h` |
| Cassini invariant | `\|φ·y1 + ψ·y2 + 1\| < 0.1` | `src/config/gf_n_encryption.h` |

### What is Standard Assumptions

| Property | Assumption |
|----------|-----------|
| CKKS ciphertext security | Ring-LWE (IND-CPA) |
| TFHE gate security | TFHE scheme security |
| GF-N key secrecy | Symmetric cipher |

### What is NOT Claimed

- ❌ Quantum resistance (CKKS/TFHE are lattice-based, quantum-sensitive)
- ❌ Formal NIST certification
- ❌ Perfect correctness (CKKS is approximate)
- ❌ Side-channel immunity (software-only defense, hardware TEE needed)

---

## Verified Test Results

| Test | Result | Source File |
|------|--------|-------------|
| FHE 10,000 cycles | 9.51 c/s, 0.01% warnings | `test_fhe_10k_fixed.cpp` |
| iO 1M gates | 10.18s, PASS | `test_io_tfhe_1m_sparse.cpp` |
| iO 100 gates XOR | 4/4 correct | `test_io_tfhe_100gates.cpp` |
| DualGate bridge | CKKS→TFHE PASS | `test_bridge_simple.cpp` |
| Serialization | 44.8MB, recovered 0.42 | `test_serialization_fixed.cpp` |

---

## Threat Model

### Attacker Capabilities (Assumed)

- Full access to source code
- Full access to ciphertexts
- Ability to run arbitrary FHE operations
- Quantum computing capabilities (for lattice attacks)

### Attacker Cannot (Based on Architecture)

- **Extract plaintext during bootstrap:** The bootstrap decrypts to GF-N intermediate, not plaintext. Source: `src/fhe/decrypt_layer.h`
- **Distinguish circuits of same size:** Encrypted coefficients under TFHE. Source: `src/io/spiral_io_tfhe.h`
- **Break GF-N without key:** Multi-layer encryption with y2_trail. Source: `src/config/gf_n_encryption.h`

### What Attacker CAN Do

- **Lattice attacks on CKKS/TFHE ciphertexts:** These rely on standard Ring-LWE hardness. If Ring-LWE breaks, ciphertexts are vulnerable. This is an inherent limitation of all lattice-based FHE.
- **Side-channel on untrusted server:** If the server is compromised at OS level, memory dump may reveal plaintext during computation. Mitigation: TEE (SGX/TrustZone) — not yet implemented.

---

## Limitations (Honest Disclosure)

| Limitation | Impact | Mitigation |
|-----------|--------|------------|
| CKKS approximate arithmetic | ±10^-10 error | Acceptable for statistical outputs |
| TEE not implemented | Plaintext exposure on compromised server | Hardware TEE required for production |
| OpenFHE v1.5.1 dependency | Gate mapping bugs in dev branch | Use stable release only |
| No formal audit | Regulatory compliance | Seeking academic review |

---

## Audit Status

| Date | Version | Type | Result |
|------|---------|------|--------|
| 2026-08-13 | v47.0 | Self-audit | All documented tests pass |
| Pending | — | External | Seeking academic review |

---

## Responsible Disclosure

1. Report via email
2. Acknowledgment within 48 hours
3. Investigation within severity timeframe
4. Patch released
5. Public disclosure after 30 days or mutual agreement

---

*"The security is layered: computational where necessary, structural where possible. φ·ψ = -1 is a theorem, not a conjecture."*

*— Dan Joseph M. Fernandez*
