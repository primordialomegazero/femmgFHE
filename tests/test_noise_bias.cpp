#include "../src/core/true_poly_fhe.h"
#include <iostream>
#include <iomanip>
using namespace std;

int main() {
    TruePolyFHE fhe;
    uint64_t seed = 42;
    
    cout << "=== NOISE BIAS ANALYSIS ===" << endl;
    cout << "small_noise(seed, 0, nonce) values for nonce 0-99:" << endl;
    
    int64_t sum = 0;
    for (uint64_t n = 0; n < 100; n++) {
        int64_t noise = fhe.small_noise(seed, 0, n);
        sum += noise;
        if (n < 20) cout << "  nonce=" << n << " → " << noise << endl;
    }
    cout << "  ... (100 samples)" << endl;
    cout << "  Average: " << (double)sum / 100.0 << endl;
    cout << "  Bias: " << (sum != 0 ? "YES ← ito yung off-by-one source" : "NONE") << endl;
    
    // Test encrypt/decrypt cycle for values -10 to 10
    cout << "\n=== ENC/DEC ACCURACY (-10 to 10) ===" << endl;
    int errors = 0;
    for (int v = -10; v <= 10; v++) {
        auto ct = fhe.encrypt(v, seed);
        int64_t dec = fhe.decrypt(ct, seed);
        if (dec != v) {
            cout << "  v=" << v << " → dec=" << dec << " ❌ (diff=" << (dec-v) << ")" << endl;
            errors++;
        }
    }
    if (errors == 0) cout << "  All 21 values correct ✅" << endl;
    else cout << "  " << errors << " errors found" << endl;
    
    // Test what happens with add
    cout << "\n=== ADD ACCURACY ===" << endl;
    auto a = fhe.encrypt(5, seed);
    auto b = fhe.encrypt(5, seed);
    auto c = fhe.add(a, b);
    cout << "  Enc(5)+Enc(5) decrypted = " << fhe.decrypt(c, seed) << " (expect 10)" << endl;
    
    // Trace the actual values
    cout << "  a.coeffs[0]=" << a.coeffs[0] << " b.coeffs[0]=" << b.coeffs[0] << endl;
    cout << "  c.coeffs[0]=" << c.coeffs[0] << endl;
    cout << "  noise(a)=" << fhe.small_noise(seed, 0, a.nonce) << endl;
    cout << "  noise(b)=" << fhe.small_noise(seed, 0, b.nonce) << endl;
    cout << "  noise(c)=" << fhe.small_noise(seed, 0, c.nonce) << endl;
    
    return 0;
}
