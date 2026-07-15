# Troubleshooting Guide

## Build Issues

### "openfhe.h: No such file or directory"
```bash
# OpenFHE not installed or not found
sudo ldconfig
ls /usr/local/include/openfhe/pke/openfhe.h  # Should exist
# If missing: reinstall OpenFHE
```

### "Cannot find -lOPENFHEcore"
```bash
# Library not linked
sudo ldconfig
ls /usr/local/lib/libOPENFHEcore.so  # Should exist
```

### Makefile warnings about "overriding recipe"
```
# Harmless. Ignore these. They're from duplicate targets in the Makefile.
# Build should still succeed.
```

## Runtime Issues

### "Segmentation fault (core dumped)"
```
Common causes:
1. Ring dimension too small for operations (try ring_dim >= 4096)
2. Multiplicative depth exceeded (increase SetMultiplicativeDepth)
3. Memory exhausted (reduce operations or increase RAM)
```

### "Timeout" in benchmarks
```
Heavy tests (True Divine, Absolute ZANS) take minutes to hours.
Run them individually:
  ./bin/phi_true_divine_10k          # ~50 minutes
  ./bin/phi_true_divine_10k_optimized # ~1 minute (ring dim 4096)
  ./bin/phi_absolute_zans            # ~30 seconds
```

### "Noise scale = 0" or unexpected values
```
If GetNoiseScaleDeg() returns unexpected values:
- This function is approximate. Use direct decryption to verify correctness.
- See src/core/phi_zans_direct_verify.cpp for the correct approach.
```

### Tests fail in FAST mode
```
Some tests are skipped in FAST mode. Run with --full:
  ./tests/full_blown_test.sh --full
```

## OpenSSL Issues

### "RAND_bytes was not declared"
```bash
# Missing include
# Add to your file: #include <openssl/rand.h>
```

### Deprecation warnings (OpenSSL 3.0)
```
Add to compile flags: -Wno-deprecated-declarations
These are harmless. The code works with OpenSSL 3.0.
```

## Performance Issues

### "Why is it so slow?"
```
FHE is computationally intensive. Typical speeds on Ryzen 5 2600:
- ZANS addition: ~2,800 ops/s (ring dim 16384)
- True Divine: ~3 steps/s (ring dim 16384)
- SpiralMicro KEM: ~414,000 ops/s (no FHE)

For faster testing:
- Use TOY mode (ring dim 4096): ~10x faster
- Use ./bin/phi_true_divine_10k_optimized for quick CTxCT tests
```

### "Can I use GPU?"
```
GPU support is planned. Currently CPU-only.
See docs/HARDWARE_SCALING.md for the roadmap.
```

## Questions?

- **GitHub Issues:** Bug reports and feature requests
- **Email:** djmfernandez@proton.me
- **Docs:** THEOREM.md, WHITEPAPER.md, GETTING_STARTED.md
