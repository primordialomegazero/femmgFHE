# Guidelines and Reproducibility

**Version 2.0**

---

## 1. Hardware Requirements

### Minimum

| Component | Minimum | Notes |
|-----------|---------|-------|
| CPU | 64-bit, 2 cores | Any modern x86_64 |
| RAM | 4 GB | For basic tests |
| Storage | 100 MB | Source + compiled binaries |
| OS | Linux (Ubuntu 20.04+) | macOS 12+ also works |

### Recommended

| Component | Recommended | Notes |
|-----------|-------------|-------|
| CPU | 64-bit, 8 cores, 3.5+ GHz | For batch operations |
| RAM | 16 GB | For stress tests |
| Storage | 1 GB SSD | For benchmark logs |
| OS | Linux (Ubuntu 22.04+) | Best compatibility |

### Unsupported

- 32-bit CPUs (NTL requires 64-bit)
- Windows (untested; may work with WSL)
- VPS with shared CPU (timing benchmarks unreliable)

---

## 2. Software Requirements

| Dependency | Minimum Version |
|-----------|----------------|
| GCC | 9.0 |
| Clang | 10.0 |
| NTL | 11.0 |
| GMP | 6.1 |
| Git | 2.0 |

---

## 3. Install

### Ubuntu / Debian

```bash
sudo apt-get update
sudo apt-get install libntl-dev libgmp-dev git
```

### macOS

```bash
brew install ntl gmp
```

### Arch Linux

```bash
sudo pacman -S ntl gmp
```

---

## 4. Build and Run

### Single Test

```bash
cd femmgFHE

g++ -std=c++17 -O3 -march=native -I/usr/include \
    tests/test_privacy_system.cpp \
    -o test_privacy_system -lntl -lgmp -lm

./test_privacy_system
```

### All Tests

```bash
for test in tests/test_*.cpp; do
    name=$(basename "$test" .cpp)
    g++ -std=c++17 -O3 -march=native -I/usr/include \
        "$test" -o "/tmp/$name" -lntl -lgmp -lm
    "/tmp/$name"
done
```

---

## 5. Expected Results

### test_privacy_system

```
=== FULL TEST (4 combinations) ===
  XOR(0,0) = 0 (expected 0)
  XOR(0,1) = 1 (expected 1)
  XOR(1,0) = 1 (expected 1)
  XOR(1,1) = 0 (expected 0)
```

### test_io_stress

```
Correct: 100/100
Hidden: 100/100, Max KS: 0
```

### test_adversarial

```
FINAL VERDICT: 8/8 ATTACKS BLOCKED
```

### test_full_benchmark

```
Golden iO Evaluate: ~20-29M ops/sec
Batch Encryption: ~47-60K bits/sec
```

Note: exact numbers vary by hardware.

---

## 6. Benchmark Conditions

1. Close other applications
2. Run benchmarks 3+ times
3. Report median values
4. Note CPU model and clock speed

---

## 7. Troubleshooting

| Error | Solution |
|-------|----------|
| `NTL/ZZ_pX.h: No such file` | `sudo apt-get install libntl-dev` |
| `gmp.h: No such file` | `sudo apt-get install libgmp-dev` |
| `undefined reference to ZZ_p` | Add `-lntl` |
| Segmentation fault | Reduce test size |

---

## 8. Environment Report Template

```
CPU: [model]
RAM: [size]
OS: [name]
Compiler: [version]
NTL: [version]
GMP: [version]
Commit: [hash]
```

---

## Contact

**Email:** devilswithin13@gmail.com

---

*This document is for reproducing test results at small scale (N=1024, Q=2^29). It does not cover production deployment.*
