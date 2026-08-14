# Contributing to Golden Privacy System

**Version 2.0**

---

## Table of Contents

1. [Code of Conduct](#1-code-of-conduct)
2. [Getting Started](#2-getting-started)
3. [Development Environment](#3-development-environment)
4. [Project Structure](#4-project-structure)
5. [Coding Standards](#5-coding-standards)
6. [Testing Requirements](#6-testing-requirements)
7. [Pull Request Process](#7-pull-request-process)
8. [Issue Reporting](#8-issue-reporting)
9. [Security Vulnerabilities](#9-security-vulnerabilities)
10. [Documentation](#10-documentation)
11. [Performance Requirements](#11-performance-requirements)

---

## 1. Code of Conduct

### Our Pledge

We are committed to providing a welcoming and harassment-free environment for all contributors regardless of experience level, background, identity, disability, appearance, race, ethnicity, nationality, religion, or age.

### Expected

- Welcoming and inclusive language
- Respect for differing viewpoints
- Constructive criticism acceptance
- Focus on project improvement
- Empathy toward others

### Unacceptable

- Harassment of any form
- Discriminatory language
- Trolling or insults
- Publishing private information
- Unwanted sexual attention
- Unprofessional conduct

### Enforcement

Report violations to: **devilswithin13@gmail.com**

All reports are reviewed confidentially.

---

## 2. Getting Started

### Prerequisites

| Requirement | Version |
|-------------|---------|
| C++ Compiler | GCC 9+ or Clang 10+ |
| NTL Library | 11.0+ |
| GMP Library | 6.1+ |
| Git | 2.0+ |

### Required Knowledge

- Fully Homomorphic Encryption (FHE)
- Indistinguishability Obfuscation (iO)
- Lattice-based cryptography (RLWE)
- Quantum computing fundamentals
- Golden ratio (φ = 1.618...)

### Setup

```bash
# Clone
git clone <repository-url>
cd femmgFHE

# Install dependencies (Ubuntu/Debian)
sudo apt-get update
sudo apt-get install libntl-dev libgmp-dev

# Verify build
g++ -std=c++17 -O3 -march=native -I/usr/include \
    tests/test_privacy_system.cpp \
    -o test_privacy_system -lntl -lgmp -lm
./test_privacy_system
```

---

## 3. Development Environment

### Recommended Tools

| Tool | Purpose |
|------|---------|
| VSCode / CLion / Vim | Editor |
| GDB / LLDB | Debugger |
| Valgrind / gprof / perf | Profiler |
| clang-tidy / cppcheck | Static analysis |
| Git + feature branches | Version control |

### Compiler Flags

```bash
# Debug
g++ -std=c++17 -O0 -g -Wall -Wextra -I/usr/include test.cpp -o test -lntl -lgmp -lm

# Release
g++ -std=c++17 -O3 -march=native -DNDEBUG -I/usr/include test.cpp -o test -lntl -lgmp -lm

# Sanitizers
g++ -std=c++17 -O1 -g -fsanitize=address,undefined -I/usr/include test.cpp -o test -lntl -lgmp -lm
```

---

## 4. Project Structure

```
femmgFHE/
├── src/
│   ├── golden_privacy_system.h     # Main unified API
│   ├── golden_prng.h               # Golden Angle PRNG
│   ├── golden_lucas.h              # Lucas One-Way
│   ├── golden_equidistributed.h    # Equidistributed Noise
│   ├── fhe/
│   │   ├── golden_quantum_fhe.h    # FHE core
│   │   ├── golden_bootstrapping.h  # Bootstrapping
│   │   └── ...
│   └── io/
│       ├── golden_io_exact.h       # iO core
│       └── ...
├── tests/
│   ├── test_privacy_system.cpp     # Main test
│   ├── attacks/
│   │   └── class_sss/              # SSS attack suite
│   └── ...
├── docs/
│   ├── FORMAL_PROOF.md             # 14 theorems
│   ├── API_REFERENCE.md
│   └── ...
└── README.md
```

---

## 5. Coding Standards

### Naming Conventions

| Element | Style | Example |
|---------|-------|---------|
| Classes | PascalCase | `GoldenPrivacySystem` |
| Functions | snake_case | `obfuscate_program()` |
| Variables | snake_case | `num_inputs` |
| Constants | UPPER_SNAKE_CASE | `PHI` |
| Namespaces | PascalCase | `GoldenFHE` |

### Code Style

- 4 spaces indentation (walang tabs)
- Max line length: 100 characters
- K&R braces
- Always use braces

```cpp
// Good
if (condition) {
    do_something();
}

// Bad
if (condition) do_something();
```

### Comments

```cpp
// GOOD: Explains WHY
// Use 3 components kasi s² ≠ -1 in this ring
result.c2 = NTL::ZZ_pX();

// BAD: Repeats code
// Set c2 to empty
result.c2 = NTL::ZZ_pX();
```

---

## 6. Testing Requirements

### Before Submitting

- [ ] All existing tests pass
- [ ] New tests for new functionality
- [ ] No compiler warnings
- [ ] Valgrind clean
- [ ] No performance regression

### Test Coverage

| Component | Coverage | Required Tests |
|-----------|----------|----------------|
| FHE Core | 100% | encrypt, decrypt, NAND, bootstrap |
| iO Core | 100% | obfuscate, evaluate, KS=0 |
| Circuit iO | 100% | XOR, AND, O(n) gates |
| Quantum | 100% | Hadamard, measurement |
| Golden PRNG | 100% | 1M unique, balance |
| Lucas | 100% | 0 collisions, tamper detect |
| Security | 100% | 8 attack classes |

### Test Template

```cpp
void test_function() {
    // Setup
    GoldenPrivacySystem gps(42);
    
    // Test cases
    for (bool bit : {false, true}) {
        auto ct = gps.encrypt_data(bit);
        assert(gps.decrypt_result(ct) == bit);
    }
    
    std::cout << "test_function: PASSED\n";
}
```

---

## 7. Pull Request Process

### Commit Format

```
TYPE: Description

- Specific change 1
- Specific change 2
- Performance/security impact
```

**Types:** `FEATURE:`, `FIX:`, `DOCS:`, `TEST:`, `REFACTOR:`, `PERF:`, `SECURITY:`

### PR Checklist

- [ ] Tests pass
- [ ] Documentation updated
- [ ] No performance regression
- [ ] Clear commit messages

### Review Process

1. Submit PR
2. Maintainer review (48h)
3. Address feedback
4. 1 maintainer approval
5. CI passes → Merge

---

## 8. Issue Reporting

### Bug Report

```markdown
## Bug
[Description]

## Reproduction
1. [Steps]

## Expected
[What should happen]

## Actual
[What happens]

## Environment
- OS: [version]
- Compiler: [version]
- NTL: [version]
```

### Feature Request

```markdown
## Feature
[Description]

## Use Case
[Why useful]

## Implementation
[How to do it]
```

---

## 9. Security Vulnerabilities

**DO NOT open public issue for vulnerabilities.**

Email: **devilswithin13@gmail.com**

### Response Time

| Severity | Response | Patch |
|----------|----------|-------|
| Critical | 24h | 48h |
| High | 48h | 1 week |
| Medium | 1 week | 2 weeks |
| Low | 2 weeks | Next release |

---

## 10. Documentation

### Required Per Contribution

1. Code comments (WHY)
2. API documentation
3. Test documentation
4. README update if needed

### Style

- English
- Concise but complete
- Include examples
- Mathematical foundation
- Security implications

---

## 11. Performance Requirements

### Benchmarks

| Component | Minimum | Target | Current |
|-----------|---------|--------|---------|
| iO Evaluation | 1M/s | 25M/s | **29M/s** |
| Batch Encrypt | 1K/s | 48K/s | **47.6K/s** |
| Full Pipeline | 1K/s | 29K/s | **77/s** |
| Quantum Gates | 10M/s | 203M/s | **203M/s** |

### Regression Policy

Ang performance regression >10% ay nangangailangan ng justification.

---

## Questions?

1. Read documentation
2. Check existing issues
3. Contact: **devilswithin13@gmail.com**

---

*Thank you for contributing to the Golden Privacy System.*
