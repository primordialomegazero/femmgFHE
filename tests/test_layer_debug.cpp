#include "../src/core/femmg_operations.h"
#include "../src/math/phi_polynomial.h"
#include "../src/math/anti_lattice.h"
#include <iostream>
using namespace std;

int main() {
    uint64_t seed = 42;
    
    // ======== LAYER 1: CHAOS ========
    cout << "=== LAYER 1: CHAOS TEST ===" << endl;
    FEmmgFHE fhe;
    auto ct1 = fhe.encrypt(42);
    int64_t dec1 = fhe.decrypt(ct1);
    cout << "Encrypt(42) -> Decrypt = " << dec1 << " " << (dec1 == 42 ? "OK" : "FAIL") << endl;
    cout << "ct1.value_int = " << ct1.value_int << endl;
    cout << "ct1.operations = " << ct1.operations << endl;
    cout << "ct1.integrity_tag = " << ct1.integrity_tag << endl;
    cout << "ct1.random_iv = " << ct1.random_iv << endl;
    
    // ======== LAYER 2: φ-POLYNOMIAL ROUNDTRIP ========
    cout << "\n=== LAYER 2: φ-POLY ROUNDTRIP ===" << endl;
    vector<int64_t> coeffs(phi_polynomial::DEFAULT_N, 0);
    coeffs[0] = ct1.value_int;
    coeffs[1] = static_cast<int64_t>(ct1.operations);
    coeffs[2] = static_cast<int64_t>(ct1.integrity_tag);
    coeffs[3] = static_cast<int64_t>(ct1.random_iv);
    
    cout << "Original coeffs[0]=" << coeffs[0] << endl;
    cout << "Original coeffs[1]=" << coeffs[1] << endl;
    
    phi_polynomial::PhiPoly orig(coeffs, 1LL << 30);
    auto noise = phi_polynomial::phi_noise(phi_polynomial::DEFAULT_N, 1LL << 30, seed);
    auto noised = orig + noise;
    auto recovered = noised - noise;
    
    cout << "Recovered coeffs[0]=" << recovered[0] << endl;
    cout << "Recovered coeffs[1]=" << recovered[1] << endl;
    bool poly_ok = (recovered[0] == coeffs[0] && recovered[1] == coeffs[1]);
    cout << "Poly roundtrip: " << (poly_ok ? "OK" : "FAIL") << endl;
    
    // ======== LAYER 3: ANTI-LATTICE XOR ROUNDTRIP ========
    cout << "\n=== LAYER 3: XOR ROUNDTRIP ===" << endl;
    vector<uint8_t> original_bytes;
    for (int i = 0; i < noised.degree(); i++) {
        int64_t v = noised[i];
        original_bytes.push_back((uint8_t)(v & 0xFF));
        original_bytes.push_back((uint8_t)((v >> 8) & 0xFF));
        original_bytes.push_back((uint8_t)((v >> 16) & 0xFF));
        original_bytes.push_back((uint8_t)((v >> 24) & 0xFF));
    }
    
    anti_lattice::InfoTheoreticLayer it;
    it.generate_pad(original_bytes.size(), seed);
    vector<uint8_t> encrypted(original_bytes.size());
    it.encrypt(original_bytes.data(), encrypted.data(), original_bytes.size());
    
    vector<uint8_t> decrypted_bytes(original_bytes.size());
    it.decrypt(encrypted.data(), decrypted_bytes.data(), original_bytes.size());
    
    bool xor_ok = true;
    for (size_t i = 0; i < original_bytes.size(); i++) {
        if (original_bytes[i] != decrypted_bytes[i]) {
            xor_ok = false;
            cout << "Mismatch at byte " << i << ": orig=" << (int)original_bytes[i] 
                 << " dec=" << (int)decrypted_bytes[i] << endl;
            break;
        }
    }
    cout << "XOR roundtrip: " << (xor_ok ? "OK" : "FAIL") << endl;
    
    // ======== FULL CHAIN (manual) ========
    cout << "\n=== FULL CHAIN ===" << endl;
    // Re-encode from recovered poly
    vector<int64_t> recovered_coeffs;
    for (size_t i = 0; i < decrypted_bytes.size() && recovered_coeffs.size() < (size_t)phi_polynomial::DEFAULT_N; i += 4) {
        int64_t v = (int64_t)decrypted_bytes[i] |
                   ((int64_t)decrypted_bytes[i+1] << 8) |
                   ((int64_t)decrypted_bytes[i+2] << 16) |
                   ((int64_t)decrypted_bytes[i+3] << 24);
        if (v & 0x80000000LL) v |= 0xFFFFFFFF00000000LL;
        recovered_coeffs.push_back(v);
    }
    while (recovered_coeffs.size() < (size_t)phi_polynomial::DEFAULT_N) recovered_coeffs.push_back(0);
    
    phi_polynomial::PhiPoly final_poly(recovered_coeffs, 1LL << 30);
    auto final_poly_dec = final_poly - noise;
    
    banach::NDimCiphertext final_ct{};
    final_ct.value_int = final_poly_dec[0];
    final_ct.operations = (uint64_t)final_poly_dec[1];
    final_ct.integrity_tag = (uint64_t)final_poly_dec[2];
    final_ct.random_iv = (uint64_t)final_poly_dec[3];
    
    int64_t final_dec = fhe.decrypt(final_ct);
    cout << "Full chain decrypt: " << final_dec << " " << (final_dec == 42 ? "OK" : "FAIL") << endl;
    
    return 0;
}
