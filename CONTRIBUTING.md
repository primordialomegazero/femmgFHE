# Contributing to femmG-FHE

## Development Principles

1. **Zero External Dependencies** — Pure C++17 + POSIX syscalls only
2. **Lock-Free Architecture** — No mutexes. Atomics only.
3. **Single Liquid Endpoint** — All API through `/manifest`
4. **φ-First Design** — All algorithms must be φ-grounded

## Build

```bash
g++ -std=c++17 -O3 -march=native -pthread -o femmg_fhe src/hydra_final.cpp
```

## Testing

All tests via API endpoints:
- `add` — Homomorphic addition
- `multiply` — Homomorphic multiplication  
- `subtract` — Homomorphic subtraction
- `fractal_chain` — Multi-party computation
- `tps` — Performance benchmark
- `party_verify` — Cross-party verification

## Code Style

- C++17 standard
- No warnings (`-Wall -Wextra`)
- Lock-free patterns only
- φ constants must use `constexpr double` with full precision

## Pull Request Process

1. Ensure zero new dependencies
2. Test all 10 API endpoints
3. TPS must not regress
4. Update README if API changes
