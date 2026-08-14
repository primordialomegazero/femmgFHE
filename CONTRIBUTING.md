# Contributing to Golden Privacy System

**Version 2.0**

---

## Code of Conduct

We are committed to providing a welcoming environment for all contributors regardless of experience, background, identity, disability, appearance, race, ethnicity, nationality, religion, or age.

### Expected

- Welcoming and inclusive language
- Respect for differing viewpoints
- Constructive criticism
- Focus on project improvement

### Unacceptable

- Harassment
- Discriminatory language
- Trolling or insults
- Publishing private information
- Unprofessional conduct

### Reporting

Report violations to: **devilswithin13@gmail.com**

---

## Getting Started

### Prerequisites

| Requirement | Version |
|-------------|---------|
| C++ Compiler | GCC 9+ or Clang 10+ |
| NTL Library | 11.0+ |
| GMP Library | 6.1+ |
| Git | 2.0+ |

### Setup

```bash
git clone <repository-url>
cd femmgFHE

# Ubuntu/Debian
sudo apt-get install libntl-dev libgmp-dev

# Verify
g++ -std=c++17 -O3 -march=native -I/usr/include \
    tests/test_privacy_system.cpp \
    -o test_privacy_system -lntl -lgmp -lm
./test_privacy_system
```

---

## Development Environment

| Tool | Purpose |
|------|---------|
| VSCode / CLion / Vim | Editor |
| GDB / LLDB | Debugger |
| Valgrind / perf | Profiler |
| clang-tidy / cppcheck | Static analysis |

### Compiler Flags

```bash
# Debug
g++ -std=c++17 -O0 -g -Wall -Wextra -I/usr/include test.cpp -o test -lntl -lgmp -lm

# Release
g++ -std=c++17 -O3 -march=native -I/usr/include test.cpp -o test -lntl -lgmp -lm

# Sanitizers
g++ -std=c++17 -O1 -g -fsanitize=address,undefined -I/usr/include test.cpp -o test -lntl -lgmp -lm
```

---

## Coding Standards

### Naming

| Element | Style | Example |
|---------|-------|---------|
| Classes | PascalCase | `GoldenPrivacySystem` |
| Functions | snake_case | `obfuscate_program()` |
| Variables | snake_case | `num_inputs` |
| Constants | UPPER_SNAKE_CASE | `PHI` |

### Style

- 4 spaces indentation
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
// Use 3 components because s² != -1 in this ring
result.c2 = NTL::ZZ_pX();

// BAD: Repeats the code
// Set c2 to empty
result.c2 = NTL::ZZ_pX();
```

---

## Testing

### Before Submitting

- [ ] All existing tests pass
- [ ] New tests for new functionality
- [ ] No compiler warnings
- [ ] Valgrind clean

### Test Template

```cpp
void test_function() {
    GoldenPrivacySystem gps(42);
    
    for (bool bit : {false, true}) {
        auto ct = gps.encrypt_data(bit);
        assert(gps.decrypt_result(ct) == bit);
    }
    
    std::cout << "test_function: PASSED\n";
}
```

---

## Pull Request Process

### Commit Format

```
TYPE: Description

- Specific change
```

**Types:** `FEATURE:`, `FIX:`, `DOCS:`, `TEST:`, `REFACTOR:`, `PERF:`, `SECURITY:`

### Process

1. Submit PR
2. Maintainer review
3. Address feedback
4. Merge after approval

---

## Issue Reporting

```markdown
## Bug
[Description]

## Reproduction
1. [Steps]

## Environment
- OS: [version]
- Compiler: [version]
- NTL: [version]
```

---

## Security Vulnerabilities

**Do not open a public issue for vulnerabilities.**

Email: **devilswithin13@gmail.com**

| Severity | Response | Patch |
|----------|----------|-------|
| Critical | 24h | 48h |
| High | 48h | 1 week |
| Medium | 1 week | 2 weeks |
| Low | 2 weeks | Next release |

---

## Contact

**Email:** devilswithin13@gmail.com

---

*Thank you for contributing.*
