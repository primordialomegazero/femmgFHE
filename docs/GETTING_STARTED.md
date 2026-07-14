# Getting Started with FEmmg-FHE v6.5

Welcome! This guide will get you from zero to running encrypted computations in under 10 minutes.

---

## What is FEmmg-FHE?

FEmmg-FHE lets you **compute on encrypted data without decrypting it.** 
You can add, multiply, and query encrypted numbers — and the results stay encrypted the whole time.

The magic is **ZANS (Zero-Anchor Noise Stabilization)** — by adding encrypted zero to ciphertexts, 
noise stays bounded without the expensive bootstrapping that other FHE systems need.

---

## Quick Start (5 minutes)

### 1. Prerequisites

```bash
# Install OpenFHE (required)
git clone https://github.com/openfheorg/openfhe-development.git
cd openfhe-development && mkdir build && cd build
cmake .. && make -j$(nproc) && sudo make install && sudo ldconfig
```

### 2. Clone & Build

```bash
git clone https://github.com/primordialomegazero/femmgFHE.git
cd femmgFHE
make all
```

### 3. Run Your First Encrypted Addition

```bash
# Run the ZANS demo — adds encrypted zero 1000 times
./bin/phi_zans_bfv
```

**Expected output:**
```
Op 1000: 42 (expected 42) ✅
Φ 1000 ZANS additions complete.
Φ Truth preserved: 42 remains 42.
```

**What just happened?** You encrypted the number 42, added Enc(0) to it 1000 times, 
and the value stayed 42. Without ZANS, the noise would have corrupted it after ~30 operations.

---

## Your First 3 Programs

### Example 1: Encrypt and Decrypt

```cpp
#include <openfhe.h>
using namespace lbcrypto;

int main() {
    // Setup
    CCParams<CryptoContextBFVRNS> params;
    params.SetPlaintextModulus(65537);
    params.SetRingDim(4096);
    
    auto cc = GenCryptoContext(params);
    cc->Enable(PKE);
    auto keys = cc->KeyGen();
    
    // Encrypt
    auto ct = cc->Encrypt(keys.publicKey, 
              cc->MakePackedPlaintext(std::vector<int64_t>{42}));
    
    // Decrypt
    Plaintext pt;
    cc->Decrypt(keys.secretKey, ct, &pt);
    std::cout << "Result: " << pt->GetPackedValue()[0] << std::endl;
    // Output: Result: 42
    
    return 0;
}
```

**Compile:**
```bash
g++ -std=c++17 -o my_test my_test.cpp \
    -I/usr/local/include/openfhe -I/usr/local/include/openfhe/core \
    -I/usr/local/include/openfhe/pke \
    -L/usr/local/lib -lOPENFHEcore -lOPENFHEpke -lssl -lcrypto
```

### Example 2: Encrypted Addition

```cpp
// Same setup as above, then:
auto ct1 = cc->Encrypt(keys.publicKey, 
              cc->MakePackedPlaintext(std::vector<int64_t>{10}));
auto ct2 = cc->Encrypt(keys.publicKey, 
              cc->MakePackedPlaintext(std::vector<int64_t>{20}));

// Add while encrypted!
auto ct_sum = cc->EvalAdd(ct1, ct2);

// Decrypt to verify
Plaintext pt;
cc->Decrypt(keys.secretKey, ct_sum, &pt);
std::cout << "10 + 20 = " << pt->GetPackedValue()[0] << std::endl;
// Output: 10 + 20 = 30
```

### Example 3: ZANS — Unlimited Additions

```cpp
// Encrypt 42
auto ct = cc->Encrypt(keys.publicKey, 
              cc->MakePackedPlaintext(std::vector<int64_t>{42}));

// Create ZANS anchor (Enc(0))
auto anchor = cc->Encrypt(keys.publicKey, 
              cc->MakePackedPlaintext(std::vector<int64_t>{0}));

// Add Enc(0) 10,000 times — noise stays bounded!
for(int i = 0; i < 10000; i++) {
    ct = cc->EvalAdd(ct, anchor);
}

// Value is still 42!
Plaintext pt;
cc->Decrypt(keys.secretKey, ct, &pt);
std::cout << "After 10000 ZANS: " << pt->GetPackedValue()[0] << std::endl;
// Output: After 10000 ZANS: 42
```

---

## Learning Path

### Beginner (Today)
1. ✅ Run `./bin/phi_zans_bfv` — see ZANS in action
2. ✅ Try the 3 examples above
3. ✅ Read `THEOREM.md` Section 1-3 (ZANS basics)

### Intermediate (This Week)
4. Run `./bin/phi_fib_zans` — Fibonacci scalar multiplication
5. Run `./bin/phi_absolute_zans` — Prime consensus ZANS
6. Read `WHITEPAPER.md` Sections 1-5

### Advanced (This Month)
7. Run `./tests/benchmark_suite.sh` — full benchmark
8. Read `docs/proofs/formal_rlwe_proof.md` — security proofs
9. Contribute: pick an issue from GitHub

---

## Common Questions

**Q: What's the difference between TOY and PRODUCTION mode?**
A: TOY uses smaller parameters for fast testing. PRODUCTION uses ring dim 32768 for real security. 
   Change in `src/core/phi_config.h`: `current_mode = Mode::PRODUCTION;`

**Q: Why does my program crash with "depth exceeded"?**
A: You're doing too many multiplications without enough depth. Increase `SetMultiplicativeDepth()` 
   or use fewer multiplications.

**Q: How do I use SpiralMicro KEM for encrypted communication?**
A: See `src/core/phi_spiralmicro_kem.cpp` — it's a standalone 32-byte post-quantum KEM.

**Q: Where are the Python bindings?**
A: Coming soon! For now, use the Go bindings in `src/spiraldb/` or call C++ directly.

**Q: I found a bug! What do I do?**
A: Open a GitHub issue with: your OS, compiler version, error message, and steps to reproduce.

---

## System Overview (Simplified)

```
FEmmg-FHE v6.5
├── Core FHE (ZANS)          ← Start here!
│   ├── Addition (unlimited)
│   ├── Multiplication (Fibonacci-ZANS)
│   └── CT×CT chains (True Divine)
├── Program Obfuscation      ← Hide what you're computing
│   └── Phantom Suite (5 modes)
├── Post-Quantum KEM         ← Encrypted communication
│   └── SpiralMicro (32 bytes)
├── Authentication           ← Who are you?
│   └── Unified Auth (6-head)
├── Encrypted Database       ← Store & query encrypted data
│   └── SpiralDB (SUM, AVG, RANGE)
└── Verification             ← Prove it worked
    ├── Verifiable FHE (audit trail)
    └── Covenant Vault (tamper-proof storage)
```

---

## Next Steps

- **Read the Whitepaper:** `WHITEPAPER.md` — full mathematical framework
- **Run all tests:** `./tests/full_blown_test.sh`
- **Benchmark:** `./tests/benchmark_suite.sh`
- **Contribute:** See `docs/CONTRIBUTOR_GUIDELINES.md`

---

*"The noise is bounded. The computation is unlimited."*
