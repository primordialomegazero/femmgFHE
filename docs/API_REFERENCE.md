# FEmmg-FHE API Reference

## Core Components

### PhiCompiler (phi_compiler.h)
Main entry point. Compiles circuits with automatic φ-clean insertion.

```cpp
PhiCompiler compiler(depth);
auto result = compiler.compile_and_run(program, initial_val, multiplier);
// result.value, result.error, result.psi_noise, result.mults, result.cleans
```

### φ-Extension Operations (all depth-free)
- `mul_X(x)` — multiply by ring element X (φ in φ-reality, ψ in ψ-reality)
- `div_X(x)` — divide by ring element X
- `add(x, y)` — element-wise addition
- `mul(x, y)` — CT×CT multiplication (4 EvalMult, 1 depth)

### Asymmetric Clean
- `asymmetric_clean(x, mul_steps, div_steps)` — kill ψ-noise
- `auto_clean(x, mults_since_clean)` — adaptive clean based on circuit depth

### Fibonacci Compression
- `build_fib_powers(base, max_fib)` — precompute Fibonacci powers
- `pow_fib(x, fib_powers, n)` — compute x × base^n in O(log N) depth

## Quick Start

```cpp
PhiCompiler C(120);
auto x = C.input(1.0);
x = C.mul(x, multiplier);
x = C.auto_clean(x, 1);
double result = C.val_phi(x);
```
