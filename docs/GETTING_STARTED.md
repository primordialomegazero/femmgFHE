# Getting Started with FEmmg-FHE

## Build

```bash
git clone https://github.com/primordialomegazero/femmgFHE.git
cd femmgFHE
make all
```

## Run Tests

```bash
# Full benchmark suite
LD_LIBRARY_PATH=./openfhe-development/build/lib:$LD_LIBRARY_PATH ./bin/test_phi_final_boss

# Compare vs standard bootstrapping
LD_LIBRARY_PATH=./openfhe-development/build/lib:$LD_LIBRARY_PATH ./bin/test_phi_vs_bootstrap

# Compiler integration demo
LD_LIBRARY_PATH=./openfhe-development/build/lib:$LD_LIBRARY_PATH ./bin/test_phi_compiler

# Production parameters (RingDim=32768)
LD_LIBRARY_PATH=./openfhe-development/build/lib:$LD_LIBRARY_PATH ./bin/test_phi_32768
```

## Basic Usage

```cpp
PhiCompiler compiler(120);    // 120 depth budget
auto x = compiler.input(1.0); // Encrypt value
x = compiler.mul(x, multiplier); // Multiply
x = compiler.auto_clean(x, 1);   // Auto φ-clean
double result = compiler.val_phi(x); // Decrypt
```
