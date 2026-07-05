#include "../src/core/triple_layer_fhe.h"
#include <iostream>
using namespace std;

int main() {
    cout << "======================================================" << endl;
    cout << "  TRIPLE-LAYER FHE TEST" << endl;
    cout << "  " << TripleLayerFHE::name() << endl;
    cout << "======================================================" << endl;
    
    int pass = 0, total = 6;
    
    TripleLayerFHE fhe;
    uint64_t seed = 42;  // Fixed seed for deterministic test
    
    // 1. Basic encrypt/decrypt
    cout << "\n1. Encrypt/Decrypt: ";
    auto ct = fhe.encrypt(42, seed);
    int64_t dec = fhe.decrypt(ct, seed);
    bool ok1 = (dec == 42);
    cout << "42 -> " << dec << " " << (ok1 ? "✅" : "❌") << endl;
    if (ok1) pass++;
    
    // 2. Different plaintext
    cout << "2. Different value: ";
    auto ct2 = fhe.encrypt(777, seed);
    int64_t dec2 = fhe.decrypt(ct2, seed);
    bool ok2 = (dec2 == 777);
    cout << "777 -> " << dec2 << " " << (ok2 ? "✅" : "❌") << endl;
    if (ok2) pass++;
    
    // 3. Negative numbers
    cout << "3. Negative: ";
    auto ct3 = fhe.encrypt(-123, seed);
    int64_t dec3 = fhe.decrypt(ct3, seed);
    bool ok3 = (dec3 == -123);
    cout << "-123 -> " << dec3 << " " << (ok3 ? "✅" : "❌") << endl;
    if (ok3) pass++;
    
    // 4. Homomorphic addition
    cout << "4. Homomorphic Add: ";
    auto cta = fhe.encrypt(100, seed);
    auto ctb = fhe.encrypt(200, seed);
    auto ctc = fhe.add(cta, ctb, seed);
    int64_t dec4 = fhe.decrypt(ctc, seed);
    bool ok4 = (dec4 == 300);
    cout << "100+200=" << dec4 << " " << (ok4 ? "✅" : "❌") << endl;
    if (ok4) pass++;
    
    // 5. Multiple additions (chaining)
    cout << "5. Chain of 5 adds: ";
    auto acc = fhe.encrypt(0, seed);
    for (int i = 0; i < 5; i++) {
        auto one = fhe.encrypt(1, seed);
        acc = fhe.add(acc, one, seed);
    }
    int64_t dec5 = fhe.decrypt(acc, seed);
    bool ok5 = (dec5 == 5);
    cout << "0+1+1+1+1+1=" << dec5 << " " << (ok5 ? "✅" : "❌") << endl;
    if (ok5) pass++;
    
    // 6. Ciphertext size
    cout << "6. Ciphertext size: ";
    auto ct6 = fhe.encrypt(42, seed);
    cout << ct6.size() << " bytes" << endl;
    bool ok6 = (ct6.size() > 0);
    if (ok6) pass++;
    
    cout << "\n======================================================" << endl;
    cout << "  TRIPLE-LAYER FHE: " << pass << "/" << total << " PASSED" << endl;
    cout << "======================================================" << endl;
    
    return (pass == total) ? 0 : 1;
}
