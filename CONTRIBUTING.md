# Contributing to Spiral Fractal iO

First off — thank you. This is a solo project that became something massive. Contributions are welcome, but they must respect the mathematical integrity of the system.

---

## Quick Start

```bash
git clone https://github.com/primordialomegazero/femmgFHE.git
cd femmgFHE
make all        # Build OpenFHE + all binaries
make quick-test # Run 5-test suite (Phi Stack, Seed Tree, Chaos, KEM, Integration)
```

---

## What You Can Contribute

| Area | Priority | Difficulty |
|------|----------|------------|
| **Language Bindings** | High | Medium |
| **Documentation & Examples** | High | Easy |
| **Test Coverage** | Medium | Medium |
| **Bug Reports** | High | Easy |
| **FHE Applications** (AES, SHA, ML, DB) | Medium | Hard |
| **Core φ-Math** | Low | Extreme |
| **Hardware Acceleration** (GPU/FPGA) | Medium | Extreme |

### Language Bindings
We have Python and C. Need:
- **Node.js / NPM** package
- **Go** module
- **Rust** crate
- **Java** library

### FHE Applications
Working examples exist for AES-128, SHA-256, DB JOIN, ML Inference. More real-world FHE applications are welcome.

### Documentation
- Tutorials
- API reference improvements
- Video walkthroughs
- Academic paper translations

---

## Development Setup

### Requirements
- **OS:** Linux (WSL2 works)
- **RAM:** 8GB minimum, 16GB recommended
- **Compiler:** GCC 11+ or Clang 14+
- **Dependencies:** OpenFHE, libsodium, libssl, sqlite3

### Build Commands

| Command | Description |
|---------|-------------|
| `make all` | Build OpenFHE + all binaries |
| `make quick-test` | Run 5-test suite (~30 seconds) |
| `make run-dev` | iO validation (RingDim 4096, ~90 min) |
| `make run-16k` | 16K Spiral Bootstrap (~24h on 16GB) |
| `make install` | System-wide install |
| `make docker` | Build Docker image |
| `make clean` | Remove binaries |

### Key Files

| File | Purpose |
|------|---------|
| `src/fhe/fhe_core.h` | CKKS FHE wrapper, DualGate struct |
| `src/io/universal_compiler.h` | iO Compiler, circuit evaluation |
| `src/refresh/spiral_bootstrap.h` | Spiral Bootstrap engine |
| `src/crypto/golden_fibonacci.h` | GF-N Encryption |
| `unified-phi-stack/phi_stack.h` | φ-ψ core math library |
| `src/api/libspiral.cpp` | Stable C API |

---

## Pull Request Process

1. **Fork** the repository
2. **Create** a feature branch: `git checkout -b feat/my-feature`
3. **Write** code with documentation
4. **Test**: `make quick-test` must pass
5. **Commit**: Clear message describing what and why
6. **Push** and open a PR against `main`
7. **Review**: I'll review within 72 hours

### PR Requirements
- [ ] All existing tests pass (`make quick-test`)
- [ ] New code is documented (WHY, not just WHAT)
- [ ] No hardcoded magic numbers — use N-configurable parameters
- [ ] KS = 0.000000 preserved (if modifying iO engine)
- [ ] License header preserved in all files

---

## Code Style

### Philosophy
- **Document WHY, not just WHAT** — every struct/function needs a header comment explaining the mathematical or architectural reason it exists
- **φ/ψ duality must be clear** in naming — `phi_project()`, `psi_val()`, `commutative_reconstruct()`
- **Compile-time verification** via `static_assert` for all invariants
- **N-configurable** — no hardcoded constants; everything parameterized

### Example

```cpp
// ═══════════════════════════════════════════════════════════════
// DUALGATE PROJECTION — Map (a,b) pair to φ-basis
// ═══════════════════════════════════════════════════════════════
// φ(a,b) = a + b·φ — Active computation output
// Used by Circuit A evaluation
inline double phi_project(double a, double b) {
    return a + b * PHI;
}
```

### Naming Conventions
- `N_` prefix = configurable parameter (e.g., `N_ring_dim`, `N_fne_layers`)
- `phi_` / `psi_` = φ/ψ related functions
- `spiral_` = Spiral Bootstrap related
- `commutative_` = Order-independent operations

---

## Mathematical Integrity

The core insight is **structural security, not computational**:
- **φ·ψ = -1** → built-in self-cancellation
- **Commutative reconstruction** → order-independent output
- **KS = 0.000000** → algebraic inevitability, not approximation

Any contribution that modifies the core φ-math or iO engine MUST preserve these properties. Run `test_io_ultra_circuit` with 1000+ gates to verify KS = 0.000000.

---

## Communication

- **Issues**: GitHub Issues for bugs and feature requests
- **Email**: `devilswithin13@gmail.com` for private inquiries
- **License questions**: Same email, subject "Spiral Fractal iO — Commercial License Request"

---

## Recognition

All contributors will be listed in the repository and acknowledged in academic citations. Significant contributions may qualify for:
- Free Pro license
- Co-authorship on papers
- Revenue sharing for major features

---

*"The security is structural, not computational. KS = 0 is inevitable, not miraculous."*
