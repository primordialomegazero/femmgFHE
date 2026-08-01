# Tests — Active & Maintained

## breakthrough/
- `test_io_batched.cpp` — Main batched iO validation (DEV/TEST/PROD modes)
- `test_io_integrated_refresh.cpp` — Integrated iO with external Refresh
- `test_hardware.cpp` — Hardware sentinel & entropy test

## Running
```bash
# Quick dev test (3 variants, 10 samples, ~30s)
./bin/test_io_batched 10 3

# Test mode (5 variants, 50 samples, ~5min)
./bin/test_io_batched 50 5

# Hardware audit
./bin/test_hardware
```
