# Reproducing FEmmg-FHE Results

## Requirements
- Ubuntu 22.04 (WSL2 or native)
- 16GB RAM minimum (32GB recommended for RingDim=32768)
- OpenSSL development libraries

## Build
```bash
git clone https://github.com/primordialomegazero/femmgFHE.git
cd femmgFHE
make all
```

## Core Benchmarks

### 1. Asymmetric Clean + Fibonacci Compression (1000+ mults)
```bash
LD_LIBRARY_PATH=./openfhe-development/build/lib:$LD_LIBRARY_PATH ./bin/test_phi_ultimate_v2
```
Expected: 1000 effective multiplications, error ~10⁻¹², noise ~10⁻⁵

### 2. φ vs CKKS Bootstrapping (head-to-head)
```bash
LD_LIBRARY_PATH=./openfhe-development/build/lib:$LD_LIBRARY_PATH ./bin/test_phi_vs_bootstrap
```
Expected: φ 2.2× faster, 1,000,000× more accurate

### 3. Complex Circuit Stress Test
```bash
LD_LIBRARY_PATH=./openfhe-development/build/lib:$LD_LIBRARY_PATH ./bin/test_phi_stress
```
Expected: 100 steps mixed ops, error ~10⁻¹³

### 4. Pre-Scaling Compensation
```bash
LD_LIBRARY_PATH=./openfhe-development/build/lib:$LD_LIBRARY_PATH ./bin/test_phi_prescale
```
Expected: Signal bounded at 1.3, normal explodes to 10¹²

### 5. φ-LWE Post-Quantum KEM
```bash
./bin/phi_kem_fixedA
```
Expected: 20/20 key exchanges passed, 3584 bytes total, ~150-bit security

### 6. SNC Verification (negative result)
```bash
LD_LIBRARY_PATH=./openfhe-development/build/lib:$LD_LIBRARY_PATH ./bin/test_snc_verify
```
Expected: BARE=SNC=SNCx5 (no noise reduction effect)

## Parameter Variations
- RingDim=4096: Default (TOY)
- RingDim=32768: `./bin/test_phi_32768`
- HEStd_128_classic: RingDim=262144 (CPU-intensive)

## Known Issues
- 12-bit KEM compression: pack/unpack lossless but decryption fails (under investigation)
- 3×3 iO composed circuits: NAND gate works standalone, composition needs depth optimization
- φ-CKKS cyclotomic rings: M%5=0 works at ILDCRTParams level, crashes OpenFHE KeyGen

## Contact
Dan Joseph M. Fernandez / Primordial Omega Zero
https://github.com/primordialomegazero
