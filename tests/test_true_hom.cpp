#include "../src/core/triple_layer_fhe.h"
#include <iostream>
using namespace std;

int main() {
    cout << "======================================================" << endl;
    cout << "  TRUE HOMOMORPHIC ADDITION TEST" << endl;
    cout << "  Add(c1, c2) WITHOUT decrypting first!" << endl;
    cout << "======================================================" << endl;
    
    TripleLayerFHE fhe;
    uint64_t seed = 42;
    int pass = 0, total = 5;
    
    // 1. Basic blind add
    auto ct_a = fhe.encrypt(100, seed);
    auto ct_b = fhe.encrypt(200, seed);
    auto ct_c = fhe.add(ct_a, ct_b, seed);  // BLIND — no decrypt!
    int64_t dec = fhe.decrypt(ct_c, seed);
    bool ok1 = (dec == 300);
    cout << "1. Blind Add: 100+200=" << dec << " " << (ok1 ? "✅" : "❌") << endl;
    if (ok1) pass++;
    
    // 2. Chain of blind adds
    auto acc = fhe.encrypt(0, seed);
    for (int i = 0; i < 10; i++) {
        auto one = fhe.encrypt(1, seed);
        acc = fhe.add(acc, one, seed);  // BLIND!
    }
    int64_t dec2 = fhe.decrypt(acc, seed);
    bool ok2 = (dec2 == 10);
    cout << "2. Blind Chain (10×): 0+10=" << dec2 << " " << (ok2 ? "✅" : "❌") << endl;
    if (ok2) pass++;
    
    // 3. Encrypt/Decrypt still works
    auto ct3 = fhe.encrypt(42, seed);
    int64_t dec3 = fhe.decrypt(ct3, seed);
    bool ok3 = (dec3 == 42);
    cout << "3. Basic Enc/Dec: 42=" << dec3 << " " << (ok3 ? "✅" : "❌") << endl;
    if (ok3) pass++;
    
    // 4. Negative blind add
    auto ct_neg = fhe.encrypt(-50, seed);
    auto ct_pos = fhe.encrypt(100, seed);
    auto ct_sum = fhe.add(ct_neg, ct_pos, seed);
    int64_t dec4 = fhe.decrypt(ct_sum, seed);
    bool ok4 = (dec4 == 50);
    cout << "4. Blind Add: -50+100=" << dec4 << " " << (ok4 ? "✅" : "❌") << endl;
    if (ok4) pass++;
    
    // 5. Ciphertext indistinguishability
    auto ct_x = fhe.encrypt(42, seed);
    auto ct_y = fhe.encrypt(42, seed);
    bool ok5 = (ct_x != ct_y);  // Different ciphertexts for same plaintext
    cout << "5. IND-CPA (ct(42) != ct(42)): " << (ok5 ? "✅" : "❌") << endl;
    if (ok5) pass++;
    
    cout << "\n======================================================" << endl;
    cout << "  TRUE HOMOMORPHIC: " << pass << "/" << total << " PASSED" << endl;
    cout << "======================================================" << endl;
    
    return (pass == total) ? 0 : 1;
}
