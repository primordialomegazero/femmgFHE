#include "../src/core/femmg_operations.h"
#include "../src/math/phi_polynomial.h"
#include <iostream>
using namespace std;

int main() {
    FEmmgFHE fhe;
    auto ct1 = fhe.encrypt(42);
    
    cout << "=== ORIGINAL VALUES ===" << endl;
    cout << "value_int = " << ct1.value_int << endl;
    cout << "operations = " << ct1.operations << endl;
    cout << "integrity_tag = " << ct1.integrity_tag << endl;
    cout << "random_iv = " << ct1.random_iv << endl;
    
    // Pack into 32-bit chunks
    auto pack = [](int64_t v) -> pair<int64_t, int64_t> {
        uint64_t u = static_cast<uint64_t>(v);
        return {static_cast<int64_t>(u & 0xFFFFFFFFULL),
                static_cast<int64_t>((u >> 32) & 0xFFFFFFFFULL)};
    };
    
    auto [v0_lo, v0_hi] = pack(ct1.value_int);
    auto [v1_lo, v1_hi] = pack(ct1.operations);
    
    cout << "\n=== PACKED ===" << endl;
    cout << "v0_lo=" << v0_lo << " v0_hi=" << v0_hi << endl;
    cout << "v1_lo=" << v1_lo << " v1_hi=" << v1_hi << endl;
    
    // Put in polynomial
    int N = phi_polynomial::DEFAULT_N;
    vector<int64_t> coeffs(N, 0);
    coeffs[0] = v0_lo; coeffs[1] = v0_hi;
    coeffs[2] = v1_lo; coeffs[3] = v1_hi;
    
    phi_polynomial::PhiPoly poly(coeffs, 1LL << 32);
    
    // Add noise
    auto noise = phi_polynomial::phi_noise(N, 1LL << 32, 42);
    auto noised = poly + noise;
    
    // Remove noise
    auto recovered = noised - noise;
    
    cout << "\n=== AFTER NOISE ROUNDTRIP ===" << endl;
    cout << "recovered[0]=" << recovered[0] << " (expected " << v0_lo << ")" << endl;
    cout << "recovered[1]=" << recovered[1] << " (expected " << v0_hi << ")" << endl;
    cout << "recovered[2]=" << recovered[2] << " (expected " << v1_lo << ")" << endl;
    cout << "recovered[3]=" << recovered[3] << " (expected " << v1_hi << ")" << endl;
    
    // Check modulo behavior
    cout << "\n=== MODULO CHECK ===" << endl;
    int64_t mod = 1LL << 32;
    cout << "noise[0]=" << noise[0] << endl;
    cout << "(poly[0] + noise[0]) % mod = " << ((poly[0] + noise[0]) % mod) << endl;
    cout << "recovered[0] raw = " << recovered[0] << endl;
    cout << "Difference from expected: " << (recovered[0] - v0_lo) << endl;
    
    // Unpack test
    auto unpack = [](int64_t lo, int64_t hi) -> int64_t {
        uint64_t u = (static_cast<uint64_t>(hi & 0xFFFFFFFFULL) << 32) |
                      static_cast<uint64_t>(lo & 0xFFFFFFFFULL);
        return static_cast<int64_t>(u);
    };
    
    int64_t val0_recovered = unpack(recovered[0], recovered[1]);
    cout << "\n=== UNPACKED ===" << endl;
    cout << "value_int recovered = " << val0_recovered << endl;
    cout << "Match original: " << (val0_recovered == ct1.value_int ? "YES" : "NO") << endl;
    
    return 0;
}
