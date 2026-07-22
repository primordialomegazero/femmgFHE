# Reproduction Guide

How to reproduce the key results from scratch.

---

## Prerequisites

- Linux (Ubuntu 20.04+ or similar)
- g++ with C++17 support
- CMake 3.14+
- 8+ GB RAM (16+ recommended)
- 5+ GB free disk space

---

## Step 1: Clone and Build OpenFHE

```bash
git clone https://github.com/primordialomegazero/femmgFHE.git
cd femmgFHE

# Build OpenFHE
cd openfhe-development
mkdir build && cd build
cmake .. -DWITH_OPENMP=OFF -DWITH_NATIVEOPT=ON
make -j$(nproc)
cd ../..
```

---

## Step 2: Build FEmmG Tests

```bash
make all
```

---

## Step 3: Run Key Tests

### Complete System Demo (quick, ~2 min)
```bash
LD_LIBRARY_PATH=./openfhe-development/build/lib:$LD_LIBRARY_PATH ./bin/test_phi_complete
```

### Clean Cycle Isolation (~1 min)
```bash
LD_LIBRARY_PATH=./openfhe-development/build/lib:$LD_LIBRARY_PATH ./bin/test_phi_clean_cycle
```

### Bootstrap-Free Depth (~2 min)
```bash
LD_LIBRARY_PATH=./openfhe-development/build/lib:$LD_LIBRARY_PATH ./bin/test_phi_bootfree_v2
```

### Asymmetric Clean Ratio Sweep (~1 min)
```bash
LD_LIBRARY_PATH=./openfhe-development/build/lib:$LD_LIBRARY_PATH ./bin/test_phi_asymmetric_clean
```

### Bootstrap + Recovery (~5 min)
```bash
LD_LIBRARY_PATH=./openfhe-development/build/lib:$LD_LIBRARY_PATH ./bin/test_phi_unlimited
```

### Gauntlet Stress Test (~30-60 min)
```bash
LD_LIBRARY_PATH=./openfhe-development/build/lib:$LD_LIBRARY_PATH ./bin/test_phi_gauntlet
```

---

## Expected Results

All tests should show:
- **φ-error** growing linearly (not exponentially)
- **ψ-noise** remaining flat at 10⁻¹²–10⁻¹⁵
- **All status checks showing ✓** (no DIVERGED)

If you see DIVERGED or CRASHED, check:
1. OpenFHE build (re-run cmake + make)
2. Available RAM (close other applications)
3. Disk space (need ~2GB free for temporary files)

---

## Verification Checklist

- [ ] `test_phi_complete` runs without errors
- [ ] `test_phi_bootfree_v2` reaches 150+ multiplications
- [ ] `test_phi_unlimited` completes 3+ bootstraps
- [ ] ψ-noise stays below 10⁻⁶ in all tests
- [ ] φ-error growth is linear (R² > 0.99)

---

## Troubleshooting

| Problem | Solution |
|---------|----------|
| "Cannot open shared object file" | Check `LD_LIBRARY_PATH` |
| "Decryption failed" during asymmetric clean | Reduce clean ratio or increase RingDim |
| Out of memory | Reduce RingDim to 4096 |
| Build errors | `make clean && make all` |
