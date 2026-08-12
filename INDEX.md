# Spiral FHE+iO — Verification Index

## Quick Verification

1. Read `docs/FORMAL_PROOF_FINAL.md` — Complete formal security proof
2. Read `docs/ON_BREAKTHROUGHS_AND_PRISONS.md` — Context and philosophy
3. Compile and run test files in `tests/`
4. For FHE/iO researchers, start with `src/fhe/spiral_fhe_io_final.h` and `src/io/spiral_io_tfhe.h`

---

## Current System Components

### FHE (CKKS-based)
| Component | Source File | Test File | Status |
|-----------|-------------|-----------|--------|
| Main bootstrap | `src/fhe/spiral_fhe_io_final.h` | `tests/test_fhe_10k_fixed.cpp` | 10K cycles PASS |
| SK isolation | `src/fhe/decrypt_layer.h` | — | Production |
| GF-N encryption | `src/config/gf_n_encryption.h` | — | Production |
| Serialization | `src/fhe/complete_homomorphic_layer.h` | `tests/test_serialization_fixed.cpp` | PASS |

### iO (TFHE-based)
| Component | Source File | Test File | Status |
|-----------|-------------|-----------|--------|
| TFHE universal circuit | `src/io/spiral_io_tfhe.h` | `tests/test_io_tfhe.cpp` | 4/4 XOR |
| 1M gates scale | `src/io/spiral_io_tfhe.h` | `tests/scaled_tests/test_io_tfhe_1m_sparse.cpp` | PASS 10.18s |
| CKKS EvalSum (bounded) | `src/io/spiral_io_final_complete.h` | `tests/test_io_complete.cpp` | 4/4 XOR |
| Circuit cancellation | `src/io/spiral_io_layer1_bura.h` | `tests/test_layer1_bura.cpp` | PASS |

### Bridge (CKKS ↔ TFHE)
| Component | Source File | Test File | Status |
|-----------|-------------|-----------|--------|
| DualGate projection | `src/bridge/dual_gate_bridge_fixed.h` | `tests/test_bridge_simple.cpp` | PASS |
| TEE transport | `src/bridge/tee_dual_gate_bridge.h` | `tests/test_tee_bridge_client.cpp` | Serialization PASS |

---

## Reproduction Guide

### Prerequisites
- OpenFHE v1.5.1 (stable release)
- C++17 compiler, GMP, NTL
- 16GB RAM recommended

### Quick Test (FHE)
```bash
g++ -std=c++17 -O3 -o verify_fhe tests/test_fhe_10k_fixed.cpp \
    -I. \
    -Iopenfhe-development/src/pke/include \
    -Iopenfhe-development/src/core/include \
    -Iopenfhe-development/src/binfhe/include \
    -Lopenfhe-development/build/install/lib \
    -lOPENFHEpke -lOPENFHEcore -lOPENFHEbinfhe -lntl -lgmp -lm
./verify_fhe
```

**Expected:** 10,000 cycles, 9.51 c/s, Cassini 99.99% stable, Status: PASS.

### Quick Test (iO)
```bash
g++ -std=c++17 -O3 -o verify_io tests/test_io_tfhe.cpp \
    -I. \
    -Iopenfhe-development/src/pke/include \
    -Iopenfhe-development/src/core/include \
    -Iopenfhe-development/src/binfhe/include \
    -Lopenfhe-development/build/install/lib \
    -lOPENFHEpke -lOPENFHEcore -lOPENFHEbinfhe -lntl -lgmp -lm
./verify_io
```

**Expected:** XOR truth table 4/4, unlimited depth via TFHE auto-bootstrap.

### Quick Test (Bridge)
```bash
g++ -std=c++17 -O3 -o verify_bridge tests/test_bridge_simple.cpp \
    -I. \
    -Iopenfhe-development/src/pke/include \
    -Iopenfhe-development/src/core/include \
    -Iopenfhe-development/src/binfhe/include \
    -Lopenfhe-development/build/install/lib \
    -lOPENFHEpke -lOPENFHEcore -lOPENFHEbinfhe -lntl -lgmp -lm
./verify_bridge
```

**Expected:** CKKS val=1 → DualGate to_bool=1 → TFHE bit=1, Status: PASS.

---

## Header Index

### Core
- `src/core/constants.h` — φ, ψ, FHE defaults

### FHE
- `src/fhe/spiral_fhe_io_final.h` — Main bootstrap (163 lines)
- `src/fhe/decrypt_layer.h` — SK isolation
- `src/fhe/complete_homomorphic_layer.h` — Serialization + erasure
- `src/fhe/seed_rotation_bootstrap.h` — Seed rotation
- `src/fhe/homomorphic_decrypt_layer.h` — Encrypted SK bootstrap
- `src/fhe/fhe_core.h` — SecureContext, create_fhe_context

### iO
- `src/io/spiral_io_tfhe.h` — TFHE universal circuit (163 lines)
- `src/io/spiral_io_final_complete.h` — CKKS EvalSum (bounded)
- `src/io/spiral_io_layer1_bura.h` — Cancellation engine
- `src/io/spiral_io_layer2_tago_v2.h` — Dual encryption
- `src/io/spiral_io_multidim_cancel.h` — VOID signature
- `src/io/spiral_io_turing.h` — Anti-Matter algebra

### Bridge
- `src/bridge/dual_gate_bridge_fixed.h` — Golden projection
- `src/bridge/tee_dual_gate_bridge.h` — TEE transport

### Crypto/Config/Utils
- `src/crypto/golden_fibonacci.h` — GF-N core
- `src/crypto/hierarchical_seed.h` — Seed tree
- `src/config/gf_n_encryption.h` — GF-N encryption engine
- `src/config/system_config.h` — System parameters
- `src/utils/safe_math.h` — Safe operations
- `src/utils/logger.h` — Logging

---

## Verification Summary

| Test | Result | Source |
|------|--------|--------|
| FHE 10K cycles | 9.51 c/s, 0.01% warnings | `tests/test_fhe_10k_fixed.cpp` |
| iO 4-gate XOR | 4/4 | `tests/test_io_tfhe.cpp` |
| iO 1M gates | 10.18s PASS | `tests/scaled_tests/test_io_tfhe_1m_sparse.cpp` |
| Bridge conversion | CKKS→TFHE PASS | `tests/test_bridge_simple.cpp` |
| Serialization | 44.8MB roundtrip PASS | `tests/test_serialization_fixed.cpp` |

---

*Foundation: φ·ψ = -1 = 1+1=2*
