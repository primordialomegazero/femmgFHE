# IO + Quantum Update Plan

## Current Issues
1. golden_io_bootstrap.h - May manual bootstrapping (decrypt-reencrypt)
2. golden_io_orbit.h - Complex number encoding, not using Fibonacci structure
3. golden_bootstrapping.h - Bootstrapper class obsolete na
4. golden_enterprise_quantum.h - Multi-layer with bootstrapping
5. golden_quantum_fhe.h - 3-component ciphertext, not using Lucas relinearization

## Target State
1. All use Fibonacci no-bootstrapping core
2. IO uses golden_plain encoding (Q/φ)
3. Quantum layers use automatic relinearization (α=L(k), β=-1)
4. No decrypt-reencrypt anywhere
5. Single unified core: golden_fibonacci_fhe.h

## Priority
1. Deprecate golden_bootstrapping.h → replace with golden_fibonacci_fhe.h
2. Update golden_quantum_fhe.h → use Fibonacci core
3. Update golden_io_bootstrap.h → remove bootstrapping
4. Update golden_io_orbit.h → integrate with Fibonacci encoding
5. Update golden_enterprise_quantum.h → use new core
