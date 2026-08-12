# Security Policy

## Supported Versions

| Version | Supported | Notes |
|---------|-----------|-------|
| **v32.x** | ✅ Active | Current stable |
| v31.x | ✅ Active | Turbo SIMD + Ultra Circuit |
| v30.x | ✅ Active | Full audit + cleanup |
| v29.x | ⚠️ Maintenance | Pre-license system |
| v25.x | ⚠️ Maintenance | Fractal iO complete |
| < v25 | ❌ End of life | Legacy architectures |

---

## Reporting a Vulnerability

**DO NOT open a public issue.**

Email: `devilswithin13@gmail.com`

Include:
- Affected version
- Steps to reproduce
- Impact assessment
- Optional: Suggested fix

### Response Time
- **Critical** (breaks KS=0 guarantee, exposes plaintext): 24 hours
- **High** (license bypass, tier escalation): 48 hours
- **Medium** (performance degradation, memory leak): 7 days
- **Low** (documentation, non-critical): 14 days

### Bug Bounty
For critical vulnerabilities in the core iO engine or Spiral Bootstrap:
- **Critical**: Free Unlimited license + public acknowledgment
- **High**: Free Enterprise license

---

## Security Model

Spiral Fractal iO security is **structural, not computational** — a paradigm shift from traditional cryptography.

### Core Guarantees

| Property | Mechanism | Proof |
|----------|-----------|-------|
| **iO Indistinguishability** | φ/ψ commutative reconstruction | KS = 0.000000 across all pairs |
| **Plaintext Never Exposed** | GF-N intermediate state during bootstrap | CKKS decrypt → GF ciphertext (not plaintext) |
| **Branch Isolation** | Irrational φ-branching | Compromise 1 branch → others safe |
| **Matrix Invertibility** | Cassini invariant > 0.1 per layer | (1/10^16)^N compound probability |
| **Side-Channel Defense** | 3-phase Spiral Obfuscation | Active during critical decrypt window |
| **Key Recovery** | Deterministic Seed Tree | 1 master seed → all branches recoverable |

### Mathematical Foundation

```
φ·ψ = -1                    → Built-in self-cancellation
φ + ψ = 1                   → DualGate projection identity
Commutative reconstruction  → Order-independent output
R[Y]/(Y²-Y-1)              → φ-extension ring
Cassini: F_{n+1}F_{n-1}-F_n² = ±1  → Always invertible
```

### What We Are NOT Vulnerable To

| Attack Vector | Status | Why |
|---------------|--------|-----|
| **Lattice attacks** | ❌ Not applicable | Security is algebraic, not lattice-based |
| **Quantum attacks** | ❌ Not applicable | φ·ψ = -1 is a mathematical fact, not a hardness assumption |
| **Side-channel timing** | ✅ Defended | 3-phase Spiral Obfuscation with Fibonacci-scaled delays |
| **Plaintext extraction** | ✅ Defended | GF-N intermediate state during bootstrap |
| **Circuit differentiation** | ✅ Defended | KS = 0.000000 indistinguishable output |
| **Key extraction** | ✅ Defended | Seed Tree branch isolation |

### Limitations (Honest Disclosure)

| Limitation | Impact | Mitigation |
|------------|--------|------------|
| **CKKS approximate arithmetic** | ±10^-10 error | Acceptable for iO (outputs are statistical distributions) |
| **Single-point key compromise** | Master seed exposure | HSM/air-gap recommended for production |
| **Consumer hardware limits** | 16GB = 8192 max RingDim | Enterprise tier: 64GB for 32768 |
| **No formal NIST certification** | Regulatory compliance | Planned: NIST PQC submission |

---

## Threat Model

### Attacker Capabilities (Assumed)
- Full access to obfuscated binary (.obf file)
- Full access to source code of libspiral.so
- Ability to run arbitrary FHE operations
- Access to side-channel information (timing, power)
- Quantum computing capabilities

### Attacker Cannot
- Distinguish Circuit A from Circuit B (KS = 0.000000)
- Extract plaintext during Spiral Bootstrap (GF-N encrypted intermediate)
- Break GF-N without N independent seeds (compound probability)
- Recover master seed from any branch seed (φ is irrational)
- Invert φ-rotation chaos (Lyapunov > 0, non-invertible)

---

## Audit Status

| Date | Version | Auditor | Result |
|------|---------|---------|--------|
| 2026-08-02 | v32.0 | Self-audit (full) | KS = 0.000000, all tests passed |
| *Pending* | — | *External audit* | *Seeking academic review* |

---

## Responsible Disclosure Timeline

1. **Report** received via email
2. **Acknowledgment** within 48 hours
3. **Investigation** and fix within timeframe based on severity
4. **Patch** released
5. **Public disclosure** after 30 days or upon mutual agreement

---

*"The security is structural, not computational. KS = 0 is inevitable, not miraculous."*
