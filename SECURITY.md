# Security Policy

## Security Model

femmG-FHE uses a **dual-layer security model**:

1. **φ-Chaotic Irreversibility** — The encryption is based on the hardness of reversing φ-chaotic orbits
2. **Ring-LWE Compatibility** — Can be combined with standard lattice-based hardness assumptions

## Triple Anti-Matter Shield

The API is protected by three security layers:

1. **φ-Harmonic Rate Limiter** — Request timing based on golden ratio (φ⁻¹ = 0.618s minimum interval)
2. **Lyapunov Anomaly Detector** — Chaotic stability analysis (threshold: 0.4812)
3. **Schumann Resonance Verification** — Earth frequency check (7.83 Hz) — bots cannot replicate

## Reporting Vulnerabilities

Please report security issues to the GitHub repository's issue tracker.

## Supported Versions

| Version | Supported |
|---------|-----------|
| v1.0.x  | ✅ Active |

## PQC Status

All 8 NIST Post-Quantum Cryptography algorithms are declared and available:
- ML-KEM-1024 (NIST Level 5)
- ML-KEM-512 (NIST Level 1)
- FrodoKEM-1344 (NIST Level 5)
- BIKE-L5 (NIST Level 5)
- ML-DSA-87 (NIST Level 5)
- Falcon-1024 (NIST Level 5)
- MAYO-5 (NIST Level 3)
- cross-rsdp-256 (NIST Level 5)

> **Note:** PQC algorithms are declared for system architecture. Full integration with φ-FHE key exchange is planned for v2.0.
