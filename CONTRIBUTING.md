# Contributing to Golden Privacy System

## Welcome

This document provides complete guidelines for contributing to the Golden Privacy System. It covers everything from setting up your development environment to submitting your first contribution. Please read this entire document before starting any contribution.

---

## Table of Contents

1. [Code of Conduct](#code-of-conduct)
2. [Getting Started](#getting-started)
3. [Development Environment](#development-environment)
4. [Project Structure](#project-structure)
5. [Coding Standards](#coding-standards)
6. [Testing Requirements](#testing-requirements)
7. [Pull Request Process](#pull-request-process)
8. [Issue Reporting](#issue-reporting)
9. [Security Vulnerabilities](#security-vulnerabilities)
10. [Documentation](#documentation)

---

## Code of Conduct

### Our Pledge

We are committed to providing a welcoming and harassment-free environment for all contributors regardless of:
- Experience level
- Educational background
- Gender identity and expression
- Sexual orientation
- Disability
- Physical appearance
- Race, ethnicity, or nationality
- Religion
- Age

### Expected Behavior

- Use welcoming and inclusive language
- Respect differing viewpoints and experiences
- Accept constructive criticism gracefully
- Focus on what is best for the project
- Show empathy toward other contributors

### Unacceptable Behavior

- Harassment in any form
- Discriminatory jokes or language
- Trolling, insulting, or derogatory comments
- Publishing others' private information without permission
- Unwanted sexual attention or advances
- Any other conduct inappropriate for a professional setting

### Enforcement

Violations may be reported to: devilswithin13@gmail.com

All reports will be reviewed and investigated confidentially.

---

## Getting Started

### Prerequisites

Before contributing, ensure you have:

1. **C++17 compiler** (GCC 9+ or Clang 10+)
2. **NTL library** installed
3. **GMP library** installed
4. **Git** configured with your GitHub account
5. **Basic understanding** of:
   - Fully Homomorphic Encryption (FHE)
   - Indistinguishability Obfuscation (iO)
   - Lattice-based cryptography
   - Quantum computing fundamentals
   - The golden ratio (φ = 1.618...)

### Setting Up

```bash
# Clone the repository
git clone <repository-url>
cd femmgFHE

# Install dependencies (Ubuntu/Debian)
sudo apt-get update
sudo apt-get install libntl-dev libgmp-dev

# Verify build works
g++ -std=c++17 -O3 -march=native -I/usr/include tests/test_privacy_system.cpp -o test_privacy_system -lntl -lgmp -lm
./test_privacy_system
```

---

## Development Environment

### Recommended Tools

- **Editor:** VSCode, CLion, or Vim
- **Debugger:** GDB or LLDB
- **Profiler:** Valgrind, gprof, or perf
- **Static Analysis:** clang-tidy, cppcheck
- **Version Control:** Git with feature branches

### Compiler Flags

```bash
# Debug build
g++ -std=c++17 -O0 -g -Wall -Wextra -I/usr/include test.cpp -o test -lntl -lgmp -lm

# Release build
g++ -std=c++17 -O3 -march=native -DNDEBUG -I/usr/include test.cpp -o test -lntl -lgmp -lm

# With sanitizers
g++ -std=c++17 -O1 -g -fsanitize=address,undefined -I/usr/include test.cpp -o test -lntl -lgmp -lm
```

---

## Project Structure

```
femmgFHE/
├── src/
│   ├── golden_privacy_system.h     # Main unified API
│   ├── fhe/                        # FHE components
│   │   ├── golden_quantum_fhe.h    # FHE core
│   │   ├── golden_bootstrapping.h  # Bootstrapping
│   │   ├── golden_relinearization.h
│   │   ├── golden_key_switching.h
│   │   ├── golden_crt_batching.h
│   │   └── ...
│   ├── io/                         # iO components
│   │   ├── golden_io_exact.h
│   │   ├── golden_io_arbitrary.h
│   │   └── golden_io_bootstrap.h
│   └── golden_lwe/
│       └── golden_lwe_opt.h
├── tests/                          # Test suite
│   ├── test_privacy_system.cpp
│   ├── test_io_stress.cpp
│   ├── test_adversarial.cpp
│   ├── test_benchmark_comparison.cpp
│   └── ...
├── docs/                           # Documentation
├── README.md
├── FORMAL_PROOF.md
├── CONTRIBUTING.md
└── LICENSE.md
```

---

## Coding Standards

### Naming Conventions

```cpp
// Classes: PascalCase
class GoldenPrivacySystem { };

// Functions/Methods: snake_case
void obfuscate_program();
bool evaluate_iO();

// Variables: snake_case
int num_inputs;
double golden_ratio;

// Constants: UPPER_SNAKE_CASE
constexpr double PHI = 1.6180339887498948482;
constexpr int MAX_DEPTH = 64;

// Namespaces: PascalCase
namespace GoldenFHE { }
namespace GoldenIO { }
```

### Code Style

- 4 spaces for indentation (no tabs)
- Maximum line length: 100 characters
- Braces on same line (K&R style)
- One statement per line
- Always use braces for control structures

```cpp
// Good
if (condition) {
    do_something();
} else {
    do_other();
}

// Bad
if (condition) do_something();
```

### Comments

- Use English for code comments
- Explain "why" not "what"
- Update comments when code changes
- Document all public APIs

```cpp
// Good: Explains why
// Use 3 components because s^2 term is not exactly -1 in this ring
Cipher result;
result.c2 = NTL::ZZ_pX();

// Bad: Just repeats the code
// Set c2 to empty polynomial
Cipher result;
result.c2 = NTL::ZZ_pX();
```

### Error Handling

- Use exceptions for unrecoverable errors
- Return error codes for expected failures
- Validate all inputs
- Document error conditions

---

## Testing Requirements

### Before Submitting

All contributions MUST:

1. Pass all existing tests
2. Include new tests for new functionality
3. Not introduce compiler warnings
4. Not introduce memory leaks (valgrind clean)
5. Maintain or improve performance

### Test Coverage

| Component | Minimum Coverage | Required Tests |
|-----------|-----------------|----------------|
| FHE Core | 100% | Encryption, decryption, NAND, bootstrapping |
| iO Core | 100% | Obfuscation, evaluation, indistinguishability |
| Quantum | 100% | Hadamard, CNOT, measurement |
| Pipeline | 100% | Full integration, batch operations |
| Security | 100% | All 7 attack classes |

### Writing Tests

```cpp
// Test structure
#include <iostream>
#include <cassert>

void test_encryption_decryption() {
    GoldenFHE::init_ring();
    
    GoldenFHE::PublicKey pk;
    GoldenFHE::SecretKey sk;
    GoldenFHE::keygen(pk, sk, 42);
    
    // Test all cases
    for (bool bit : {false, true}) {
        auto ct = GoldenFHE::encrypt(pk, bit, 1000);
        bool result = GoldenFHE::decrypt(ct, sk);
        assert(result == bit);
    }
    
    std::cout << "test_encryption_decryption: PASSED\n";
}
```

### Running Tests

```bash
# Individual test
./test_privacy_system

# All tests
for test in test_*; do
    echo "Running $test..."
    ./$test
done
```

---

## Pull Request Process

### Before Creating PR

1. Ensure your code passes all tests
2. Run valgrind to check for memory leaks
3. Format your code consistently
4. Update documentation
5. Write clear commit messages

### Commit Messages

Format: `TYPE: Description`

Types:
- `FEATURE:` New functionality
- `FIX:` Bug fix
- `DOCS:` Documentation changes
- `TEST:` Test additions/changes
- `REFACTOR:` Code restructuring
- `PERF:` Performance improvement
- `SECURITY:` Security-related changes

Example:
```
FEATURE: Add batch encryption support for 128 bits per ciphertext

- Implement CRT-style batching
- Add batch_encrypt and batch_decrypt methods
- Update tests to cover batch operations
- Performance: 48K ops/sec (142x speedup)
```

### PR Template

```markdown
## Description
[Explain what this PR does]

## Changes Made
- [List specific changes]

## Tests Run
- [List tests that pass]

## Performance Impact
- [Any performance changes]

## Security Impact
- [Any security implications]
```

### Review Process

1. Submit PR
2. Maintainer reviews within 48 hours
3. Address feedback
4. Approval required from at least 1 maintainer
5. Merge after CI passes

---

## Issue Reporting

### Bug Report Template

```markdown
## Bug Description
[Clear description of the bug]

## Reproduction Steps
1. [Step 1]
2. [Step 2]
3. [Step 3]

## Expected Behavior
[What should happen]

## Actual Behavior
[What actually happens]

## Environment
- OS: [e.g., Ubuntu 22.04]
- Compiler: [e.g., GCC 12.2]
- NTL version: [e.g., 11.5.1]
- GMP version: [e.g., 6.2.1]
```

### Feature Request Template

```markdown
## Feature Description
[What feature would you like?]

## Use Case
[Why is this useful?]

## Proposed Implementation
[How would you implement this?]

## Alternatives Considered
[What alternatives exist?]
```

---

## Security Vulnerabilities

### Reporting

**DO NOT open a public issue for security vulnerabilities.**

Email: devilswithin13@gmail.com

Include:
- Description of vulnerability
- Reproduction steps
- Impact assessment
- Suggested fix (if any)

### Response Time

- Acknowledgment: 24 hours
- Initial assessment: 72 hours
- Patch: 1-2 weeks depending on severity

### Severity Levels

| Level | Response | Patch |
|-------|----------|-------|
| Critical | 24 hours | 48 hours |
| High | 48 hours | 1 week |
| Medium | 1 week | 2 weeks |
| Low | 2 weeks | Next release |

---

## Documentation

### Required Documentation

Every contribution MUST include:

1. **Code comments** explaining the logic
2. **API documentation** for new methods
3. **Test documentation** explaining test cases
4. **README updates** if behavior changes

### Documentation Style

- Use English
- Be concise but complete
- Include examples
- Explain mathematical foundations
- Document security implications

---

## Performance Requirements

### Benchmarks

Contributions should not degrade performance without justification:

| Component | Minimum | Target |
|-----------|---------|--------|
| iO Evaluation | 1M ops/sec | 25M ops/sec |
| FHE Encryption | 100 ops/sec | 48K ops/sec (batch) |
| Full Pipeline | 1K ops/sec | 29K ops/sec |
| Quantum Gates | 10M ops/sec | 203M ops/sec |

### Benchmarking

```bash
# Run benchmarks
./test_benchmark_comparison

# Profile
gprof ./test_privacy_system gmon.out > profile.txt
```

---

## Questions?

If you have questions:

1. Check existing issues and documentation
2. Read the FORMAL_PROOF.md for security analysis
3. Read the README.md for overview
4. Contact: devilswithin13@gmail.com

---

*Thank you for contributing to the Golden Privacy System. Your work helps advance the field of privacy-preserving computation.*
