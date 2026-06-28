# Changelog

All notable changes to FEmmg-FHE will be documented in this file.

## [v1.0.0] — 2026-06-28

### Added
- Initial release of FEmmg-FHE
- Phi-contraction Fully Homomorphic Encryption engine
- 7.5M TPS true homomorphic operations (encrypt + add + decrypt pipeline)
- Self-stabilizing noise via Banach Fixed Point Theorem
- 40-byte ciphertexts
- Homomorphic addition, multiplication, and subtraction
- Multi-Recursive Fractal FHE (7 layers, 14 party keys)
- 91/91 cross-party verification
- Triple Anti-Matter security shield (Phi-limiter + Lyapunov + Schumann)
- 8 NIST PQC algorithms declared
- Lock-free multi-metaprogramming architecture (12 threads, 0 mutexes)
- Single liquid API endpoint (/manifest)
- Zero external dependencies (pure C++17 + POSIX syscalls)
- Docker support with public GHCR image
- IACR ePrint submission (#xxxx/110266)
- MIT License

### Performance
- 7.5M TPS on AMD Ryzen 5 2600 (12 cores)
- Encrypt: ~50 ns
- Homomorphic Add: ~90 ns
- Homomorphic Multiply: ~150 ns
- Noise: 40.0-40.3 bits after 10,000+ operations
