# Contributing to FEmmg-FHE

## 🌟 Welcome, Builder

FEmmg-FHE is a mathematical breakthrough in fully homomorphic encryption.
We welcome contributions that advance privacy-preserving computation.

---

## 📐 Philosophy

This project operates on three principles:

1. **Mathematics First** — Every implementation must have a formal proof backing it
2. **Zero Bootstrapping** — Noise converges via φ⁻¹ Banach contraction, not bootstrapping
3. **Chaos-Based Security** — Security derives from chaotic dynamical systems, not lattice assumptions

---

## 🚀 Getting Started

### Prerequisites

```bash
# Ubuntu/Debian
sudo apt install build-essential libssl-dev liboqs-dev

# Clone
git clone https://github.com/primordialomegazero/femmgFHE.git
cd femmgFHE

# Build and test
make test
./run_full_test_suite.sh
```

### Project Structure

```
femmgFHE/
├── src/
│   ├── core/       # LyapunovFHE, TruePolyFHE, Chaos Engine
│   ├── chaos/      # MMCA, ZSCI, SRFL, LCA
│   ├── security/   # AERS, AIC, Memory Guard, ZKP, Guardian
│   ├── math/       # φ-polynomials, Riemann zeta
│   ├── kem/        # ML-KEM-1024, φ-PKE
│   ├── server/     # Enterprise API
│   └── storage/    # SpiralDB
├── tests/          # 34K+ test suite
├── docs/
│   ├── proofs/     # Formal mathematical proofs
│   ├── api/        # API documentation
│   └── test_results/ # Automated test reports
├── paper/          # Academic paper (IACR submitted)
└── bindings/       # Python, NPM
```

---

## 🔬 Areas for Contribution

### 1. Formal Verification (High Priority)
- Machine-checked proofs in Coq/Lean
- Noise convergence proofs
- IND-CPA reduction proofs

### 2. Performance Optimization
- AVX-512 polynomial multiplication
- GPU acceleration (CUDA/OpenCL)
- Distributed FHE (MPI)

### 3. Security Analysis
- Third-party cryptanalysis
- Side-channel audit
- Formal security reduction to chaos assumptions

### 4. Language Bindings
- Rust (pyo3 alternative)
- WebAssembly
- Java/Kotlin

### 5. Applications
- Private ML inference
- Encrypted database queries
- Secure multi-party computation

---

## 📝 Contribution Process

### 1. Discuss First
Open an issue describing your proposed change. Tag with:
- `proof` — New theorem or formal verification
- `optimization` — Performance improvement
- `security` — Cryptanalysis or hardening
- `binding` — Language support
- `application` — Use case implementation

### 2. Branch & Code
```bash
git checkout -b feature/your-feature
# Make changes
# Add tests (REQUIRED)
```

### 3. Test Everything
```bash
./run_full_test_suite.sh
# Must pass all 34K+ tests
```

### 4. Document
- Update `docs/proofs/` if adding new mathematics
- Update `docs/api/` if changing the API
- Add inline comments for complex algorithms

### 5. Pull Request
- Reference the issue number
- Include test results
- Describe the mathematical basis
- Note any security implications

---

## 🧪 Testing Standards

### Required Tests
- Unit tests for new functions
- Integration tests for layer interactions
- Security tests for IND-CPA/CCA2
- Performance benchmarks (no regressions)

### Running Tests
```bash
# Individual test
g++ -std=c++17 -O0 -o build/test_xxx tests/test_xxx.cpp -lm -lssl -lcrypto
./build/test_xxx

# Full suite
./run_full_test_suite.sh

# With report
cat docs/test_results/test_report_latest.md
```

---

## 📚 Documentation Standards

### Code Comments
```cpp
// ═══ SECTION HEADER ═══
// Algorithm description
// Complexity: O(n)
// Reference: Theorem X in docs/proofs/
```

### Commit Messages
```
component: brief description

Detailed explanation of what changed and why.
Reference theorems or issues.
```

### Mathematical Notation
Use LaTeX in documentation:
```
$$\text{noise}_{k+1} = \text{noise}_k^2 \cdot \phi^{-1}$$
```

---

## 🛡️ Security Disclosures

For security vulnerabilities, please **DO NOT** open a public issue.

Email: `security@femmg-fhe.io` (if available) or use GitHub's private vulnerability reporting.

We follow responsible disclosure:
- Acknowledgment within 48 hours
- Patch within 7 days
- Public disclosure after patch
- CVE request if applicable

---

## ⚖️ License

MIT License — see [LICENSE](LICENSE)

By contributing, you agree that your contributions will be licensed under MIT.

---

## 🙏 Acknowledgments

- **Dan Joseph M. Fernandez / Primordial Omega Zero** — Original author
- **Lyapunov, Banach, Fibonacci** — Mathematical foundations
- **Gentry (2009)** — Pioneered FHE; this work eliminates bootstrapping

---

> *"The optimal contraction reveals the weakness of computational infinity."* — φΩ0
