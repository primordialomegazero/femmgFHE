# DM-DGR Tests

## Directory Structure

### `final/` — Production-Ready DM-DGR Tests
- `test_phi_dm_dgr_final.cpp` — **THE UNIFIED SYSTEM**: Complete arbitrary CT computation
- `test_phi_dm_dgr_unified.cpp` — DM-DGR unified cycle test
- `test_phi_basic_check.cpp` — Basic operation debug tests
- `phi_kem_level5.c` — NIST Level 5 KEM
- `phi_kem_qr.c` — QR-code KEM

### `crosslib/` — Cross-Library Validation
- `test_phi_seal_crosslib.cpp` — SEAL validation
- `test_phi_helib_crosslib.cpp` — HElib validation
- `test_phi_tfhe_crosslib.cpp` — TFHE validation
- `test_phi_crosslib_final.cpp` — 9/9 compatibility matrix
- `test_phi_full_crosslib.cpp` — Full cross-library test

### `legacy/` — Historical Tests (Archived)
Earlier experiments and intermediate tests.
Not needed for current DM-DGR operation.

## Quick Run

```bash
# Build and run the final system
cd ..
g++ -std=c++17 -O3 -march=native \
  -I./openfhe-development/src/pke/include \
  -I./openfhe-development/src/core/include \
  -I./openfhe-development/src/binfhe/include \
  -I./openfhe-development/build/src/core \
  -o bin/test_phi_dm_dgr_final \
  tests/final/test_phi_dm_dgr_final.cpp \
  -L./openfhe-development/build/lib \
  -lOPENFHEpke -lOPENFHEcore -lOPENFHEbinfhe \
  -Wl,-rpath,./openfhe-development/build/lib \
  -lstdc++ -lpthread -lm

LD_LIBRARY_PATH=./openfhe-development/build/lib:$LD_LIBRARY_PATH ./bin/test_phi_dm_dgr_final
