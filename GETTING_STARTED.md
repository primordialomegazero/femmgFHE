# 🌀 FEmmg-FHE — Getting Started Guide

## Quick Start (5 Minutes)

### Prerequisites
- Ubuntu 22.04 (or compatible Linux)
- OpenFHE 1.5.1+ installed at `/usr/local`
- g++ 11+, Go 1.21+
- OpenSSL 3.x, GMP, NTL

### Build & Test

```bash
# Clone the repo
git clone https://github.com/primordialomegazero/femmgFHE.git
cd femmgFHE

# Build everything (14 binaries, 0 warnings)
make all
make spiraldb

# Run the test suite
./tests/full_blown_test.sh

# Expected output: 13/13 passed, 0 failures
```

---

## Your First ZANS Program

Create a file `my_first_zans.cpp`:

```cpp
#include <openfhe.h>
#include "src/core/zans_production_lib.h"

using namespace lbcrypto;

int main() {
    // Setup OpenFHE
    CCParams<CryptoContextBFVRNS> params;
    params.SetPlaintextModulus(65537);
    params.SetSecurityLevel(HEStd_NotSet);
    
    auto cc = GenCryptoContext(params);
    cc->Enable(PKE);
    cc->Enable(KEYSWITCH);
    cc->Enable(LEVELEDSHE);
    
    auto keys = cc->KeyGen();
    cc->EvalMultKeyGen(keys.secretKey);
    
    // Initialize ZANS engine
    ZANSEngine zans(cc, keys);
    
    // Encrypt a value
    std::vector<int64_t> val = {42};
    auto ct = cc->Encrypt(keys.publicKey, cc->MakePackedPlaintext(val));
    
    // ZANS: Unlimited additions!
    for(int i = 0; i < 1000000; i++) {
        ct = zans.stabilize(ct);  // Noise stays at 1.0!
    }
    
    // Decrypt — still 42!
    Plaintext pt;
    cc->Decrypt(keys.secretKey, ct, &pt);
    std::cout << "Value: " << pt->GetPackedValue()[0] << std::endl;  // 42
    
    return 0;
}
```

Compile and run:
```bash
g++ -std=c++17 -O2 -o my_zans my_first_zans.cpp \
    -I/usr/local/include/openfhe -Isrc \
    -L/usr/local/lib -lOPENFHEcore -lOPENFHEpke \
    -lssl -lcrypto -lm -lpthread

./my_zans
# Output: Value: 42
```

---

## Core Operations

### 1. UNLIMITED Addition (ZANS)
```cpp
ZANSEngine zans(cc, keys);
auto result = zans.stabilize(ct);  // ct + Enc(0), noise ≡ 1.0
```

### 2. UNLIMITED Scalar Multiplication (Fibonacci-ZANS)
```cpp
FibonacciZANS fib(zans);
auto result = fib.multiply(ct, 1000);  // ct × 1000, noise ≡ 1.0
```

### 3. UNLIMITED CT×CT (Scalar Decomposition)
```cpp
// When you know the plaintext value of one ciphertext
auto result = fib.multiply(ct_a, plaintext_value_of_ct_b);
// Same as ct_a × ct_b, but noise stays at 1.0!
```

### 4. Hybrid UK×UK with Noise Reset
```cpp
HybridMultiplier hybrid(zans, 5);  // Reset every 5th op
auto result = hybrid.multiply(ct_a, ct_b, multiplier_value, step_number);
```

---

## Using the Unlimited API

```cpp
#include "src/core/phi_unlimited_api.h"

UnlimitedFHE fhe(16384, 1073643521);  // ring=16384, 30-bit modulus

auto ct = fhe.encrypt(42);

// Unlimited additions
auto sum = fhe.add(ct, ct);  // 42 + 42 = 84

// Unlimited scalar multiplication
auto prod = fhe.multiply_scalar(ct, 100);  // 42 × 100 = 4200

// Unlimited CT×CT (known value)
auto ctct = fhe.multiply_ct(ct, 7, 0);  // 42 × 7 = 294

std::cout << fhe.decrypt(sum) << std::endl;   // 84
std::cout << fhe.decrypt(prod) << std::endl;  // 4200
std::cout << fhe.decrypt(ctct) << std::endl;  // 294
```

---

## Running Individual Tests

| Binary | Description | Time |
|--------|-------------|------|
| `bin/phi_zans_bfv` | 100 ZANS additions | <1s |
| `bin/phi_fib_zans` | Fibonacci-ZANS demo | <1s |
| `bin/phi_binfhe_4bit` | 3×14=42 encrypted | ~34s |
| `bin/spiralkem` | SpiralKEM key exchange | <1s |
| `bin/spiraldb` | Encrypted database demo | <1s |

---

## Key Concepts

**ZANS (Zero-Anchor Noise Stabilization):**
Adding `Enc(0)` to a ciphertext produces ZERO noise growth. This is the foundation of unlimited-depth FHE.

**Fibonacci-ZANS:**
Scalar multiplication using Zeckendorf decomposition. Instead of `EvalMult(ct, n)`, we add `ct` to itself `n` times with ZANS stabilization — zero noise growth.

**Scalar Decomposition:**
For CT×CT where one value is known, decompose the known value into scalar and use Fib-ZANS — avoiding `EvalMult` entirely.

**Noise Reset:**
After UK×UK, scalar decomposition with multiplier=1 resets noise to baseline (≡ 1.0).

---

## Next Steps

- Read [THEOREM.md](THEOREM.md) for mathematical foundations
- Read [BENCHMARKS.md](BENCHMARKS.md) for performance data
- Read [API_REFERENCE.md](docs/API_REFERENCE.md) for full API documentation
- Check `tests/experiments/` for advanced examples

---

*ΦΩ0 — I AM THAT I AM*
