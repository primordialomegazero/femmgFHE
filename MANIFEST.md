# FEmmG-FHE Source Tree Manifest

## KEEP (Production)
- `src/femmg/phi_core.h` — FHE core library
- `src/io/phi_io_core.h` — iO core library
- `src/io/phi_io_compiler.h` — iO compiler
- `src/kem/phi_kem_level5.c` — NIST Level 5 KEM (192 bytes)
- `src/kem/phi_kem_qr.c` — QR-code KEM (80 bytes)
- `src/kem/phi_kem_ultra_v2.c` — Ultra-compact KEM (1792 bytes)
- `src/kem/phi_kem_v5.c` — Nano KEM (128 bytes)
- `src/kem/phi_kem_v5.h` — KEM API header
- `tests/active/*.cpp` — Active test suite (18 tests)
- `docs/*.md` — Documentation
- `paper/` — Research paper draft
- `Makefile` — Build system
- `README.md` — Main entry point
- `LICENSE` — MIT

## KEEP (Reference)
- `openfhe-development/` — OpenFHE library (build only)
- `libs/` — External libraries (HydraJWT, libsodium-schnorr)

## ARCHIVE (Keep for history, not needed for build)
- `archive/` — All legacy experiments, docs, backups
- `tests/archive/` — Old test files

## CUT (Can be removed)
- `SEAL/` — Old SEAL library (migrated to OpenFHE)
- `build_verify/` — One-off verification builds
- `results/` — Old test output files
- `scripts/` — Misc scripts (check if any are useful)
- `examples/` — Old examples (check if any are current)
- `bindings/` — Python/Go/Java/Rust bindings (keep if working)
- `src/binfhe/`, `src/core/`, `src/integration/`, `src/snark/`, `src/spiraldb/`, `src/transmute/`, `src/zkp/` — Legacy implementations
- `femmgfhe/` — Duplicate directory?

## DECISION NEEDED
- `bin/` — Compiled binaries (can rebuild from source)
- `*.cpp` and `*.h` in root — Check if any are active
