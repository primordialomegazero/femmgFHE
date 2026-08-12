# Spiral FHE+iO — Verification Index

## Quick Verification
1. Read `docs/FORMAL_PROOF_FINAL.md` — The complete formal security proof
2. Read `docs/ON_BREAKTHROUGHS_AND_PRISONS.md` — Context and philosophy
3. Compile and run any test file in `tests/`
4. Read `src/core/README.md` for header index

## For FHE/iO Researchers
- Main header: `spiral_fractal_turing_complete.h`
- Core constants: `src/core/constants.h`
- All subsystem headers: `src/fhe/`, `src/refresh/`, `src/crypto/`, `src/io/`

## Reproduction
```bash
g++ -std=c++17 -O3 -I. -Iopenfhe-development/src/pke/include \
    -Iopenfhe-development/src/core/include \
    -o verify tests/t1_parser/test_final_compiler.cpp \
    -Lopenfhe-development/build/lib \
    -lOPENFHEpke -lOPENFHEcore -lntl -lgmp -lm
./verify
```

Expected output: 20/20 PERFECT. KS = 0.000000. All ZKP VERIFIED.
