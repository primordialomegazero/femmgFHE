# Guidelines and Reproducibility

**Version 1.0**

---

## 1. Hardware Requirements

### 1.1 Minimum Hardware

| Component | Minimum | Notes |
|-----------|---------|-------|
| CPU | 64-bit, 2 cores, 2.0 GHz | Any modern x86_64 |
| RAM | 4 GB | For basic tests |
| Storage | 100 MB | Source + compiled binaries |
| OS | Linux (Ubuntu 20.04+) | macOS 12+ also works |

### 1.2 Recommended Hardware

| Component | Recommended | Notes |
|-----------|-------------|-------|
| CPU | 64-bit, 8 cores, 3.5+ GHz | For batch operations |
| RAM | 16 GB | For stress tests |
| Storage | 1 GB SSD | For benchmark logs |
| OS | Linux (Ubuntu 22.04+) | Best compatibility |

### 1.3 Tested Hardware

| CPU | RAM | OS | Status |
|-----|-----|-----|--------|
| AMD Ryzen (8 cores) | 16 GB | Ubuntu 22.04 | All tests pass |
| Intel Core i7 (4 cores) | 8 GB | Ubuntu 20.04 | All tests pass |
| Apple M1 (8 cores) | 8 GB | macOS 13 | All tests pass |

### 1.4 Unsupported Hardware

- 32-bit CPUs (NTL requires 64-bit)
- ARM without NEON (performance issues)
- Windows (untested, may need WSL)
- VPS with shared CPU (timing benchmarks unreliable)

---

## 2. Software Requirements

### 2.1 Compiler

| Compiler | Minimum Version | Recommended |
|----------|----------------|-------------|
| GCC | 9.0 | 12.0+ |
| Clang | 10.0 | 16.0+ |

### 2.2 Libraries

| Library | Minimum Version | Ubuntu Package | macOS (brew) |
|---------|----------------|----------------|---------------|
| NTL | 11.0 | `libntl-dev` | `ntl` |
| GMP | 6.1 | `libgmp-dev` | `gmp` |

### 2.3 Build Tools

| Tool | Version | Purpose |
|------|---------|---------|
| CMake | 3.10+ | Optional build system |
| Make | 4.0+ | Alternative build |
| Git | 2.0+ | Version control |

---

## 3. Installation Guide

### 3.1 Ubuntu/Debian

```bash
# Update package list
sudo apt-get update

# Install dependencies
sudo apt-get install -y libntl-dev libgmp-dev git

# Verify installation
dpkg -l | grep ntl
dpkg -l | grep gmp
```

### 3.2 macOS

```bash
# Install Homebrew if not installed
/bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"

# Install dependencies
brew install ntl gmp

# Verify
brew info ntl
brew info gmp
```

### 3.3 Arch Linux

```bash
# Install dependencies
sudo pacman -S ntl gmp

# Verify
pacman -Q ntl gmp
```

---

## 4. Building the System

### 4.1 Single Test

```bash
# Navigate to project
cd femmgFHE

# Compile test
g++ -std=c++17 -O3 -march=native -I/usr/include \
    tests/test_privacy_system.cpp \
    -o test_privacy_system \
    -lntl -lgmp -lm

# Run
./test_privacy_system
```

### 4.2 All Tests

```bash
#!/bin/bash
# Build and run all tests

cd femmgFHE

COMPILE="g++ -std=c++17 -O3 -march=native -I/usr/include"
LIBS="-lntl -lgmp -lm"

# Build each test
for test in tests/test_*.cpp; do
    name=$(basename $test .cpp)
    echo "Building $name..."
    $COMPILE $test -o $name $LIBS
done

# Run each test
for binary in test_*; do
    echo "Running $binary..."
    ./$binary
    echo ""
done
```

### 4.3 CMake Build

```cmake
# CMakeLists.txt
cmake_minimum_required(VERSION 3.10)
project(GoldenPrivacySystem)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -march=native")
set(CMAKE_BUILD_TYPE Release)

find_package(NTL REQUIRED)
find_package(GMP REQUIRED)

add_executable(test_privacy_system tests/test_privacy_system.cpp)
target_link_libraries(test_privacy_system NTL GMP m)
```

---

## 5. Reproducing Test Results

### 5.1 Test: Privacy System

```bash
# Expected output: 4/4 XOR cases pass
./test_privacy_system
```

**Expected Results:**
```
XOR(0,0) = 0 (expected 0)
XOR(0,1) = 1 (expected 1)
XOR(1,0) = 1 (expected 1)
XOR(1,1) = 0 (expected 0)
```

### 5.2 Test: iO Stress

```bash
# Expected output: 100/100 functions correct, KS=0
./test_io_stress
```

**Expected Results:**
```
STRESS TEST 1: Correct: 100/100
STRESS TEST 3: Hidden: 100/100, Max KS: 0
STRESS TEST 4: 16M evals/sec
```

### 5.3 Test: Adversarial

```bash
# Expected output: 6/7 attacks blocked
./test_adversarial
```

**Expected Results:**
```
Zeroizing: BLOCKED
Timing: RESISTANT
Statistical: BLOCKED
Known Plaintext: BLOCKED
Quantum: BLOCKED
Side-Channel: BLOCKED
```

### 5.4 Test: Benchmark

```bash
# Expected output: 25M iO ops/sec, 48K batch ops/sec
./test_benchmark_comparison
```

**Expected Results:**
```
Golden iO: 25,000,000 ops/sec
Batch Encryption: 48,000 ops/sec
Full Pipeline: 29,000 ops/sec
```

---

## 6. Performance Reproducibility

### 6.1 Benchmark Conditions

For reproducible benchmarks:

1. **Close all other applications**
2. **Use CPU performance governor**
3. **Run benchmarks 3+ times**
4. **Report median, not mean**
5. **Note CPU model and clock speed**

```bash
# Set CPU governor to performance (Linux)
sudo cpupower frequency-set -g performance

# Run benchmark
./test_benchmark_comparison

# Restore governor
sudo cpupower frequency-set -g ondemand
```

### 6.2 Timing Measurement

```cpp
// Correct timing methodology
auto start = std::chrono::high_resolution_clock::now();

// ... operations ...

auto end = std::chrono::high_resolution_clock::now();
double duration = std::chrono::duration<double>(end - start).count();
```

### 6.3 Expected Performance Variance

| Component | Expected Variance | Cause |
|-----------|------------------|-------|
| iO Evaluation | ±5% | Cache misses, OS scheduling |
| FHE Encryption | ±20% | NTL polynomial allocation |
| Batch Operations | ±10% | Memory bandwidth |
| Quantum Gates | ±2% | Minimal (pure arithmetic) |

---

## 7. Troubleshooting

### 7.1 Compilation Errors

| Error | Cause | Solution |
|-------|-------|----------|
| `NTL/ZZ_pX.h: No such file` | NTL not installed | `sudo apt-get install libntl-dev` |
| `gmp.h: No such file` | GMP not installed | `sudo apt-get install libgmp-dev` |
| `std::complex` not found | Wrong C++ standard | Add `-std=c++17` |
| `undefined reference to ZZ_p` | Missing NTL link | Add `-lntl` |

### 7.2 Runtime Errors

| Error | Cause | Solution |
|-------|-------|----------|
| `Segmentation fault` | Memory overflow | Increase RAM or reduce test size |
| `Assertion failed` | Logic error | Check seed values |
| `Aborted (core dumped)` | Unhandled exception | Add try-catch blocks |
| `Out of memory` | Batch too large | Reduce batch size |

### 7.3 Performance Issues

| Issue | Cause | Solution |
|-------|-------|----------|
| Slow encryption | NTL overhead | Use batch encryption |
| High variance | CPU throttling | Set performance governor |
| Slow bootstrapping | Polynomial operations | Use smaller N or optimize |
| Memory leaks | NTL allocation | Use `valgrind` to identify |

---

## 8. Reproducibility Checklist

### Before Reporting Results

- [ ] Record exact compiler version: `g++ --version`
- [ ] Record NTL version: `dpkg -l | grep ntl`
- [ ] Record GMP version: `dpkg -l | grep gmp`
- [ ] Record CPU model: `lscpu | grep "Model name"`
- [ ] Record RAM: `free -h`
- [ ] Record OS: `uname -a`
- [ ] Record commit hash: `git rev-parse HEAD`
- [ ] Run tests 3+ times
- [ ] Report median values
- [ ] Note any deviations

### Environment Report Template

```
=== ENVIRONMENT REPORT ===
CPU: [model]
RAM: [size]
OS: [name and version]
Compiler: [version]
NTL: [version]
GMP: [version]
Commit: [hash]

=== TEST RESULTS ===
Test 1: [result]
Test 2: [result]
...

=== DEVIATIONS ===
[Any deviations from expected results]
```

---

## 9. Common Mistakes

### 9.1 Don't Do This

```cpp
// Wrong: Using rand() instead of mt19937
int seed = rand();  // Not reproducible

// Wrong: Not initializing NTL ring
GoldenFHE::Cipher ct;  // Will crash without init_ring()

// Wrong: Using different seeds for encryption
auto ct1 = encrypt(pk, bit, 1000);  // Nonce 1000
auto ct2 = encrypt(pk, bit, 1000);  // Same nonce = same ciphertext!
```

### 9.2 Do This Instead

```cpp
// Correct: Use fixed seed for reproducibility
std::mt19937 rng(42);  // Reproducible

// Correct: Initialize ring before using
GoldenFHE::init_ring();

// Correct: Use different nonces
auto ct1 = encrypt(pk, bit, 1000);  // Nonce 1000
auto ct2 = encrypt(pk, bit, 1001);  // Different nonce
```

---

## 10. Version Control

### 10.1 Recommended Git Workflow

```bash
# Clone
git clone <repository>
cd femmgFHE

# Create branch
git checkout -b feature/my-change

# Make changes
# ...

# Test
./test_privacy_system

# Commit
git add .
git commit -m "FEATURE: Description"

# Push
git push origin feature/my-change
```

### 10.2 Reproducible Builds

```bash
# Record environment
git describe --tags > build_info.txt
g++ --version >> build_info.txt
dpkg -l | grep ntl >> build_info.txt

# Build with fixed flags
g++ -std=c++17 -O3 -march=native -fno-fast-math \
    tests/test_privacy_system.cpp \
    -o test_privacy_system \
    -lntl -lgmp -lm
```

---

## 11. Contact

For reproducibility issues, bugs, or questions:

**Email:** devilswithin13@gmail.com

Include your environment report (Section 8) for faster resolution.

---

*These guidelines ensure that all results can be independently reproduced. Follow them carefully.*
