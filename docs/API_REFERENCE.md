# FZDB API Reference

## Core Class: `FZDB`

```cpp
#include "fzdb.h"

phi::FZDB fhe(ringDim, modulus, scale);
```

### Methods

| Method | Description |
|--------|-------------|
| `calibrate()` | One-time setup (computes correction constants) |
| `phiEncode(msg)` | Encode message m → Enc(m × φ) |
| `phiDecode(ct)` | Decode ciphertext → plaintext |
| `refresh(ct)` | FZDB refresh — pure homomorphic, no decrypt |
| `add(a, b)` | Homomorphic addition |
| `multiply(a, b)` | Homomorphic multiplication |
| `multiplyScalar(ct, n)` | Multiply by known constant |
| `noiseLevel(ct)` | Get noise scale degree |

### Example

```cpp
phi::FZDB fhe;
auto a = fhe.phiEncode(42);
auto b = fhe.phiEncode(58);
auto sum = fhe.add(a, b);
auto prod = fhe.multiply(a, b);
auto refreshed = fhe.refresh(prod);
std::cout << fhe.phiDecode(sum) << std::endl;  // 100
std::cout << fhe.phiDecode(refreshed) << std::endl;  // 2436
```
