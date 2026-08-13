# Spiral Golden Framework

**Complete Homomorphic Encryption + Private Function Evaluation + Indistinguishability Obfuscation**

## Source Structure

```
src/
├── fhe/                               FHE Production
│   ├── golden_quantum_fhe.h           Main FHE core
│   ├── golden_enterprise_quantum.h    Enterprise FHE (4-layer)
│   ├── golden_quantum_fhe_simd.h      SIMD optimized
│   └── golden_quantum_fhe_opt.h       Parallel optimized
│
├── io/                                iO Production
│   ├── golden_io_exact.h              FGG multilinear iO
│   └── golden_io_arbitrary.h          Arbitrary function iO
│
├── quantum/                           Quantum Gates
│   ├── golden_fractal_quantum.h       Fractal Hadamard/CNOT
│   └── golden_quantum_milestones.h    Bell, GHZ, QFT
│
├── mpc/                               Multi-Party Computation
│   └── golden_mpc.h                   Threshold + ZKP
│
└── audit/                             Audit Trail
    └── golden_audit_logger.h          Immutable audit
```

## Foundation

**φ·ψ = -1 = 1+1=2**

## Features

- FHE: Unlimited depth bootstrap
- PFE: TFHE universal circuit
- iO: FGG multilinear maps (arbitrary functions)
- Quantum: Hadamard, CNOT, Bell states
- MPC: Threshold decryption
- ZKP: Fiat-Shamir proofs
- Audit: Immutable trail

## Build

```bash
# FHE
g++ -std=c++17 -O3 -mavx2 -pthread -o test_fhe tests/test_enterprise_quantum_stress.cpp \
    -I. -Iopenfhe-development/src/core/include \
    -Iopenfhe-development/src/pke/include \
    -Iopenfhe-development/src/binfhe/include \
    -Lopenfhe-development/build/install/lib \
    -lOPENFHEpke -lOPENFHEcore -lOPENFHEbinfhe -lntl -lgmp -lm

# iO
g++ -std=c++17 -O3 -o test_io tests/test_io_arbitrary.cpp \
    -I. -Iopenfhe-development/src/core/include \
    -Iopenfhe-development/src/pke/include \
    -Lopenfhe-development/build/install/lib \
    -lOPENFHEpke -lOPENFHEcore -lntl -lgmp -lm
```

## License

Custom dual license — see LICENSE.md

## Citation

```bibtex
@software{fernandez2026golden,
  author = {Dan Joseph M. Fernandez},
  title = {Spiral Golden Framework},
  year = {2026},
  version = {49.0},
  url = {https://github.com/primordialomegazero/femmgFHE}
}
```
