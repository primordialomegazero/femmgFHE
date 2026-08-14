# API Reference — Golden Privacy System

**Version 2.0**

---

## GoldenPrivacySystem

### Constructor

```cpp
GoldenPrivacySystem(uint64_t seed = 42)
```

**Description:** Initializes the system with FHE key generation, quantum state in |0⟩, Golden Angle PRNG, Lucas One-Way, Equidistributed Noise, and batch precomputation.

**Parameters:**
| Parameter | Type | Description |
|-----------|------|-------------|
| `seed` | `uint64_t` | Key generation seed (default: 42) |

**Example:**
```cpp
GoldenPrivacySystem gps(42);
```

---

## iO Operations

### obfuscate_program

```cpp
void obfuscate_program(
    const std::function<bool(const std::vector<bool>&)>& func,
    int num_inputs
)
```

**Description:** Obfuscates a function using Golden Orbit encoding (complex phases on the unit circle).

**Parameters:**
| Parameter | Type | Description |
|-----------|------|-------------|
| `func` | `std::function` | Function to obfuscate |
| `num_inputs` | `int` | Number of input bits (≥ 1) |

**Complexity:** O(2^n) time and space.

**Properties (tested at small scale):**
- KS distance = 0 (indistinguishability)
- |value| = 1 (no zero values)

**Example:**
```cpp
auto xor_func = [](const std::vector<bool>& in) {
    return in[0] ^ in[1];
};
gps.obfuscate_program(xor_func, 2);
```

---

### obfuscate_circuit_begin

```cpp
void obfuscate_circuit_begin(int num_inputs)
```

**Description:** Begins circuit-based obfuscation using NAND gates. Polynomial size (O(n) gates) instead of exponential truth table.

**Parameters:**
| Parameter | Type | Description |
|-----------|------|-------------|
| `num_inputs` | `int` | Number of circuit inputs |

**Example:**
```cpp
gps.obfuscate_circuit_begin(4);
int xor_ab = gps.circuit_add_xor(0, 1);
int xor_cd = gps.circuit_add_xor(2, 3);
int result = gps.circuit_add_xor(xor_ab, xor_cd);
```

---

### circuit_add_nand

```cpp
int circuit_add_nand(int in1, int in2)
```

**Description:** Adds a NAND gate to the circuit.

**Returns:** `int` — Output wire index.

---

### circuit_add_xor

```cpp
int circuit_add_xor(int a, int b)
```

**Description:** Adds an XOR gate (implemented as 4 NAND gates).

**Returns:** `int` — Output wire index.

---

### evaluate_io_public

```cpp
bool evaluate_io_public(const std::vector<bool>& input) const
```

**Description:** Evaluates the obfuscated program (truth table or circuit mode).

**Returns:** `bool` — Function output.

---

## FHE Operations

### encrypt_data

```cpp
GoldenFHE::Cipher encrypt_data(bool bit, uint64_t nonce = 0)
```

**Description:** Encrypts a single bit using RLWE with golden ratio scaling (Q/φ).

**Parameters:**
| Parameter | Type | Description |
|-----------|------|-------------|
| `bit` | `bool` | Plaintext bit |
| `nonce` | `uint64_t` | Nonce (0 = auto via Golden Angle PRNG) |

**Returns:** `GoldenFHE::Cipher` — 3-component ciphertext (c0, c1, c2).

**Note:** This is a research implementation. Polynomial multiplication uses naive O(N²) — no NTT.

---

### batch_encrypt

```cpp
GoldenFHE::Cipher batch_encrypt(const std::vector<bool>& bits)
```

**Description:** Encrypts up to 1024 bits in a single ciphertext by packing into polynomial coefficients.

**Returns:** `GoldenFHE::Cipher` — Batch ciphertext.

---

### decrypt_result

```cpp
bool decrypt_result(const GoldenFHE::Cipher& ct)
```

**Description:** Decrypts a ciphertext to obtain the plaintext bit.

**Returns:** `bool` — Plaintext bit.

**Complexity:** O(N²) due to s² multiplication.

---

### batch_decrypt

```cpp
std::vector<bool> batch_decrypt(
    const GoldenFHE::Cipher& ct,
    int num_bits
)
```

**Description:** Decrypts a batch ciphertext to multiple bits.

---

## Lucas Operations

### commit_value

```cpp
long long commit_value(long long value)
```

**Description:** Creates a Lucas-based commitment: `Lucas(n) = φ^n + ψ^n mod p`.

**Returns:** `long long` — Commitment.

---

### verify_commitment

```cpp
bool verify_commitment(long long value, long long commitment)
```

**Description:** Verifies a Lucas commitment.

**Returns:** `bool` — True if valid.

---

## Quantum Operations

### apply_quantum_gate

```cpp
void apply_quantum_gate()
```

**Description:** Applies Hadamard gate to the internal qubit state.

---

### measure_quantum

```cpp
double measure_quantum()
```

**Description:** Measures the quantum state.

**Returns:** `double` — P(0) in [0, 1].

---

## Full Pipeline

### compute

```cpp
GoldenFHE::Cipher compute(
    const GoldenFHE::Cipher& enc_a,
    const GoldenFHE::Cipher& enc_b
)
```

**Description:** Runs the pipeline: FHE decrypt → iO evaluate → Quantum verify → FHE re-encrypt.

**Returns:** `GoldenFHE::Cipher` — Encrypted result.

---

### batch_compute

```cpp
std::vector<bool> batch_compute(
    const std::vector<std::pair<bool, bool>>& inputs
)
```

**Description:** Batch iO evaluation over multiple input pairs.

---

## Metrics & Security

### print_metrics

```cpp
void print_metrics() const
```

**Output:** Performance counters for each layer.

### print_security

```cpp
void print_security() const
```

**Output:** Security property checks.

### get_security

```cpp
SecurityProof get_security() const
```

**Returns:**
```cpp
struct SecurityProof {
    bool fhe_ind_cpa;           // RLWE semantic security (assumed)
    bool io_indistinguishable;   // KS distance = 0 (tested)
    bool quantum_verified;       // Quantum layer active
    bool zero_test_resistant;    // No zero values (by construction)
    bool lucas_one_way;          // Collision-free (tested)
    bool prng_uniform;           // Balance 0.0002 (tested)
};
```

---

## Complete Example

```cpp
#include "src/golden_privacy_system.h"
#include <iostream>

int main() {
    GoldenPrivacySystem gps(42);
    
    // Circuit: 4-input XOR
    gps.obfuscate_circuit_begin(4);
    int xor_ab = gps.circuit_add_xor(0, 1);
    int xor_cd = gps.circuit_add_xor(2, 3);
    int xor4 = gps.circuit_add_xor(xor_ab, xor_cd);
    
    // Verify
    for (int i = 0; i < 16; i++) {
        std::vector<bool> input = {
            (bool)((i >> 3) & 1), (bool)((i >> 2) & 1),
            (bool)((i >> 1) & 1), (bool)(i & 1)
        };
        bool result = gps.evaluate_io_public(input);
        bool expected = input[0] ^ input[1] ^ input[2] ^ input[3];
        if (result != expected) return 1;
    }
    std::cout << "XOR4: 16/16 PASSED\n";
    
    return 0;
}
```

---

*This API Reference documents the research prototype as of Version 2.0. It does not claim production readiness. See ROADMAP.md for engineering work needed.*
