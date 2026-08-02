# Contributing

## Quick Start

```bash
git clone https://github.com/primordialomegazero/femmgFHE.git
cd femmgFHE
make all        # Build OpenFHE + all binaries
make quick-test # Run 5-test suite
```

## Development

- C++17 for core, C99 for KEM
- OpenFHE for homomorphic operations
- φ-ψ ring algebra for structural security

## Pull Requests

1. Fork the repo
2. Create a feature branch
3. All tests must pass: `make quick-test`
4. Update docs if needed
5. Submit PR with description

## Code Style

- Document WHY, not just WHAT
- Every struct/function needs a header comment
- Keep the φ/ψ duality clear in naming
- Use `static_assert` for compile-time verification

## Contact

Dan Joseph M. Fernandez — `devilswithin13@gmail.com`
