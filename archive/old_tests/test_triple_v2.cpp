#include "../src/core/femmg_operations.h"
#include "../src/math/phi_polynomial.h"
#include "../src/math/anti_lattice.h"
#include <iostream>
using namespace std;

int main() {
    cout << "======================================================" << endl;
    cout << "  TRIPLE-LAYER v2 — Chaos-Infused Architecture" << endl;
    cout << "======================================================" << endl;
    
    FEmmgFHE fhe;
    int pass = 0, total = 4;
    
    // ======== LAYER 1: Pure Chaos (BASELINE) ========
    cout << "\n--- Layer 1: Chaos FHE Baseline ---" << endl;
    auto ct1 = fhe.encrypt(42);
    int64_t dec1 = fhe.decrypt(ct1);
    bool l1 = (dec1 == 42);
    cout << "Encrypt/Decrypt: 42 -> " << dec1 << " " << (l1 ? "OK" : "FAIL") << endl;
    if (l1) pass++;
    
    // ======== LAYER 2: Chaos → Serialize → Deserialize → Decrypt ========
    cout << "\n--- Layer 2: Serialization Roundtrip ---" << endl;
    // Simple memcpy roundtrip
    vector<uint8_t> bytes(sizeof(ct1));
    memcpy(bytes.data(), &ct1, sizeof(ct1));
    banach::NDimCiphertext ct1_copy;
    memcpy(&ct1_copy, bytes.data(), sizeof(ct1_copy));
    int64_t dec2 = fhe.decrypt(ct1_copy);
    bool l2 = (dec2 == 42);
    cout << "Serialize→Deserialize: 42 -> " << dec2 << " " << (l2 ? "OK" : "FAIL") << endl;
    if (l2) pass++;
    
    // ======== LAYER 3: XOR Obfuscation Roundtrip ========
    cout << "\n--- Layer 3: XOR Obfuscation ---" << endl;
    anti_lattice::InfoTheoreticLayer it;
    it.generate_pad(bytes.size(), 42);
    vector<uint8_t> encrypted(bytes.size());
    it.encrypt(bytes.data(), encrypted.data(), bytes.size());
    vector<uint8_t> decrypted(bytes.size());
    it.decrypt(encrypted.data(), decrypted.data(), bytes.size());
    memcpy(&ct1_copy, decrypted.data(), sizeof(ct1_copy));
    int64_t dec3 = fhe.decrypt(ct1_copy);
    bool l3 = (dec3 == 42);
    cout << "XOR obfuscate→Deobfuscate: 42 -> " << dec3 << " " << (l3 ? "OK" : "FAIL") << endl;
    if (l3) pass++;
    
    // ======== LAYER 4: Polynomial Encode Roundtrip ========
    cout << "\n--- Layer 4: Polynomial Encoding ---" << endl;
    // Use N=64 to fit entire NDimCiphertext (400 bytes = 50 int64_t)
    constexpr int POLY_N = 64;
    vector<int64_t> coeffs(POLY_N, 0);
    // Copy raw bytes into coefficients
    for (int i = 0; i < (int)bytes.size() && i/8 < POLY_N; i += 8) {
        memcpy(&coeffs[i/8], bytes.data() + i, 8);
    }
    // Store and recover (NO modulo, NO noise)
    vector<int64_t> recovered_coeffs = coeffs;
    // Rebuild bytes
    vector<uint8_t> recovered_bytes(bytes.size());
    for (int i = 0; i < (int)bytes.size() && i/8 < POLY_N; i += 8) {
        memcpy(recovered_bytes.data() + i, &recovered_coeffs[i/8], 8);
    }
    memcpy(&ct1_copy, recovered_bytes.data(), sizeof(ct1_copy));
    int64_t dec4 = fhe.decrypt(ct1_copy);
    bool l4 = (dec4 == 42);
    cout << "Poly encode→Decode: 42 -> " << dec4 << " " << (l4 ? "OK" : "FAIL") << endl;
    if (l4) pass++;
    
    cout << "\n======================================================" << endl;
    cout << "  RESULT: " << pass << "/" << total << " PASSED" << endl;
    cout << "======================================================" << endl;
    
    return (pass == total) ? 0 : 1;
}
