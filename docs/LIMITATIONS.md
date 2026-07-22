# Honest Limitations

---

## Mathematical

| Limitation | Detail |
|------------|--------|
| Security reduction | Reduces to CKKS/RLWE security; formal proof pending |
| φ-native rings | M%5=0 cyclotomic rings crash OpenFHE KeyGen (NTT incompatibility) |
| Noise model | ψ-noise behavior empirically verified; theoretical model incomplete |

## Engineering

| Limitation | Detail |
|------------|--------|
| Bootstrap required | Clean manages noise but doesn't refresh modulus levels |
| Pre-scaling | Requires known clean schedule; adaptive scaling not yet implemented |
| Performance | ~40s per bootstrap on consumer hardware (Ryzen 5 2600) |
| Memory | RingDim=32768 requires >16GB for full bootstrap |
| Library dependence | Built on OpenFHE; not portable to SEAL or HElib without adaptation |

## Validation

| Limitation | Detail |
|------------|--------|
| Third-party review | No independent verification yet |
| Production security | Most benchmarks at TOY security (RingDim=4096/8192) |
| Circuit types | Only tested on multiplicative chains; general circuits untested |
| Comparison | No head-to-head with state-of-the-art CKKS bootstrapping at scale |

## Theoretical

| Limitation | Detail |
|------------|--------|
| φ-optimality | Is φ the optimal ring extension, or do other algebraic numbers work better? |
| Generalization | Can the asymmetric reality trick work for BFV, BGV, TFHE? |
| Lower bounds | No formal proof that linear error growth is optimal |

---

*All limitations are actively being researched. Contributions welcome.*
