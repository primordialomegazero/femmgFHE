# Getting Started with FEmmg-FHE

## Prerequisites

- Linux (Ubuntu 20.04+)
- GCC 9+ with C++17 support
- OpenFHE library (included as submodule)
- 8GB+ RAM (16GB recommended)

## Build

```bash
git clone https://github.com/primordialomegazero/femmgFHE.git
cd femmgFHE
make all
```

## Your First FHE Program

```cpp
#include "fzdb.h"

int main() {
    phi::FZDB fhe;
    
    // Encrypt two numbers
    auto a = fhe.phiEncode(10);
    auto b = fhe.phiEncode(20);
    
    // Compute encrypted
    auto sum = fhe.add(a, b);
    auto prod = fhe.multiply(a, b);
    
    // Refresh (zero-decrypt bootstrap)
    auto refreshed = fhe.refresh(prod);
    
    // Decrypt and print
    std::cout << "10 + 20 = " << fhe.phiDecode(sum) << std::endl;
    std::cout << "10 × 20 = " << fhe.phiDecode(refreshed) << std::endl;
    
    return 0;
}
```

## Running Tests

```bash
# FZDB demo
LD_LIBRARY_PATH=./openfhe-development/build/lib:$LD_LIBRARY_PATH ./bin/fzdb_demo

# TFHE benchmark
LD_LIBRARY_PATH=./openfhe-development/build/lib:$LD_LIBRARY_PATH ./bin/phi_tfhe_benchmark

# Predictive bootstrap
LD_LIBRARY_PATH=./openfhe-development/build/lib:$LD_LIBRARY_PATH ./bin/phi_path_a_predictive_test
```

## Performance Tips

- Use BFV for fast arithmetic (< 30 multiplications)
- Use TFHE for unlimited depth
- Apply FZDB refresh every ~25 operations
- For production, use ringDim=32768+
