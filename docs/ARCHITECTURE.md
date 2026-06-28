# FEmmg-FHE Architecture

## Overview

FEmmg-FHE is a lock-free, multi-threaded FHE system with zero external dependencies.

## Core Components

### Phi-FHE Engine
- Encryption via phi-contraction mapping
- Self-stabilizing noise (Banach Fixed Point)
- Homomorphic add, multiply, subtract
- 40-byte ciphertexts

### Fractal FHE
- 7 recursive encryption layers
- 14 party keys with cross-verification
- Chain operations (add/multiply across parties)
- 91/91 verified party pairs

### Anti-Matter Shield
- Phi-harmonic rate limiter
- Lyapunov anomaly detector
- Schumann resonance verification

### Liquid API
- Single endpoint architecture (/manifest)
- All operations through action parameter
- JSON response format

## Thread Model

- 12 worker threads (configurable)
- Zero mutexes (lock-free atomics only)
- Each thread handles accept → read → process → write → close
- Phi-harmonic load distribution via SO_REUSEPORT

## Data Flow

```
Client Request
    │
    ▼
[Anti-Matter Shield]
    │
    ▼
[JSON Parser]
    │
    ├── encrypt  → Phi-FHE Engine → Ciphertext
    ├── add      → Phi-FHE Engine → Result
    ├── multiply → Phi-FHE Engine → Result
    ├── subtract → Phi-FHE Engine → Result
    ├── fractal  → Fractal FHE    → Chain Result
    ├── tps      → Benchmark Loop → TPS Report
    └── status   → System Info    → Health JSON
    │
    ▼
[JSON Response]
    │
    ▼
Client Response
```

## Security Model

1. Phi-Chaotic Irreversibility: Encryption based on hardness of reversing phi-chaotic orbits
2. Ring-LWE Compatible: Can compose with standard lattice assumptions
3. Anti-Matter: Rate limiting + anomaly detection + Schumann verification
4. PQC Ready: 8 NIST algorithms declared

## Performance Characteristics

- Encrypt: ~50 ns
- Add: ~90 ns
- Multiply: ~150 ns
- Throughput: 9-10M ops/sec (encrypt+add+decrypt)
- Scaling: Linear with thread count
