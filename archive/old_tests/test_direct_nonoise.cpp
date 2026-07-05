#include "../src/core/femmg_operations.h"
#include "../src/math/phi_polynomial.h"
#include <iostream>
using namespace std;

int main() {
    FEmmgFHE fhe;
    int N = phi_polynomial::DEFAULT_N;
    
    auto pack = [](int64_t v) -> pair<int64_t, int64_t> {
        uint64_t u = static_cast<uint64_t>(v);
        return {static_cast<int64_t>(u & 0xFFFFFFFFULL),
                static_cast<int64_t>((u >> 32) & 0xFFFFFFFFULL)};
    };
    
    auto unpack = [](int64_t lo, int64_t hi) -> int64_t {
        uint64_t u_lo = static_cast<uint64_t>(lo & 0xFFFFFFFFLL);
        uint64_t u_hi = static_cast<uint64_t>(hi & 0xFFFFFFFFLL);
        return static_cast<int64_t>((u_hi << 32) | u_lo);
    };
    
    cout << "=== DIRECT (NO NOISE): Chaos → φ-Poly → Chaos ===" << endl;
    
    for (int test_val : {42, 777, -123, 0}) {
        auto ct1 = fhe.encrypt(test_val);
        
        vector<int64_t> coeffs(N, 0);
        auto [v0_lo, v0_hi] = pack(ct1.value_int);
        auto [v1_lo, v1_hi] = pack(ct1.operations);
        auto [v2_lo, v2_hi] = pack(ct1.integrity_tag);
        auto [v3_lo, v3_hi] = pack(ct1.random_iv);
        
        coeffs[0] = v0_lo; coeffs[1] = v0_hi;
        coeffs[2] = v1_lo; coeffs[3] = v1_hi;
        coeffs[4] = v2_lo; coeffs[5] = v2_hi;
        coeffs[6] = v3_lo; coeffs[7] = v3_hi;
        
        // MODULUS = 2^63 (no overflow for 32-bit chunks)
        phi_polynomial::PhiPoly poly(coeffs, 1LL << 62);
        // NO NOISE — direct roundtrip
        auto recovered = poly;  // Identity
        
        banach::NDimCiphertext rebuilt{};
        rebuilt.value_int    = unpack(recovered[0], recovered[1]);
        rebuilt.operations   = unpack(recovered[2], recovered[3]);
        rebuilt.integrity_tag = unpack(recovered[4], recovered[5]);
        rebuilt.random_iv    = unpack(recovered[6], recovered[7]);
        
        int64_t dec = fhe.decrypt(rebuilt);
        cout << test_val << " -> " << dec << " " << (dec == test_val ? "OK" : "FAIL") << endl;
    }
    
    return 0;
}
