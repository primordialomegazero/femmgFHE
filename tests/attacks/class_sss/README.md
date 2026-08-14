# Class SSS Attack Suite

Ang mga tests dito ay ang **pinaka-mabangis** na attacks para sa Golden Privacy System.

## Attacks:

1. **test_sss_bruteforce.cpp** - Full brute force (3^1024 keyspace)
2. **test_sss_lattice.cpp** - Lattice reduction (LLL/BKZ)
3. **test_sss_quantum.cpp** - Quantum attacks (Shor/Grover)
4. **test_sss_sidechannel.cpp** - Side-channel analysis (timing/cache/power)
5. **test_sss_zeroizing.cpp** - Zeroizing attacks (GGH13/CLT13 style)
6. **test_sss_adaptive.cpp** - Adaptive chosen ciphertext (CCA2)

## Expected Results:

| Attack | Result |
|--------|--------|
| Brute Force | INFEASIBLE (10^488 keys) |
| Lattice | RESISTANT (N=1024 beyond LLL) |
| Quantum | INFEASIBLE (10^244 ops) |
| Side-Channel | CONSTANT-TIME |
| Zeroizing | BLOCKED (no zero values) |
| CCA2 | BLOCKED (no oracle) |

## How to Run:

```bash
cd tests/attacks/class_sss
g++ -std=c++17 -O3 -march=native -I../../../src test_sss_*.cpp -o test_sss -lntl -lgmp -lm
./test_sss
