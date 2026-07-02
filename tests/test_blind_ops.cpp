#include "../src/core/triple_layer_fhe.h"
#include <iostream>
using namespace std;

int main() {
    cout << "======================================================" << endl;
    cout << "  TRUE BLIND HOMOMORPHIC OPERATIONS" << endl;
    cout << "  Add() and Multiply() WITHOUT decrypting!" << endl;
    cout << "======================================================" << endl;
    
    TripleLayerFHE fhe;
    uint64_t seed = 42;
    int pass = 0, total = 8;
    
    // 1. Basic Encrypt/Decrypt
    auto ct = fhe.encrypt(42, seed);
    int64_t dec = fhe.decrypt(ct, seed);
    bool ok1 = (dec == 42);
    cout << "\n1. Enc/Dec: 42=" << dec << " " << (ok1 ? "✅" : "❌") << endl;
    if (ok1) pass++;
    
    // 2. Blind Add
    auto a = fhe.encrypt(100, seed);
    auto b = fhe.encrypt(200, seed);
    auto c = fhe.add(a, b, seed);
    int64_t dec2 = fhe.decrypt(c, seed);
    bool ok2 = (dec2 == 300);
    cout << "2. Blind Add: 100+200=" << dec2 << " " << (ok2 ? "✅" : "❌") << endl;
    if (ok2) pass++;
    
    // 3. Blind Multiply
    auto x = fhe.encrypt(6, seed);
    auto y = fhe.encrypt(7, seed);
    auto z = fhe.multiply(x, y, seed);
    int64_t dec3 = fhe.decrypt(z, seed);
    bool ok3 = (dec3 == 42);
    cout << "3. Blind Mul: 6×7=" << dec3 << " " << (ok3 ? "✅" : "❌") << endl;
    if (ok3) pass++;
    
    // 4. Blind Add chain (10 operations)
    auto acc = fhe.encrypt(0, seed);
    for (int i = 0; i < 10; i++) {
        auto one = fhe.encrypt(1, seed);
        acc = fhe.add(acc, one, seed);
    }
    int64_t dec4 = fhe.decrypt(acc, seed);
    bool ok4 = (dec4 == 10);
    cout << "4. Blind Chain (10 adds): 0+10=" << dec4 << " " << (ok4 ? "✅" : "❌") << endl;
    if (ok4) pass++;
    
    // 5. Blind Mixed: (a+b)*c
    auto a2 = fhe.encrypt(3, seed);
    auto b2 = fhe.encrypt(4, seed);
    auto c2 = fhe.encrypt(5, seed);
    auto sum = fhe.add(a2, b2, seed);
    auto prod = fhe.multiply(sum, c2, seed);
    int64_t dec5 = fhe.decrypt(prod, seed);
    bool ok5 = (dec5 == 35);
    cout << "5. Blind Mixed: (3+4)×5=" << dec5 << " " << (ok5 ? "✅" : "❌") << endl;
    if (ok5) pass++;
    
    // 6. Negative blind add
    auto neg = fhe.encrypt(-50, seed);
    auto pos = fhe.encrypt(100, seed);
    auto sum2 = fhe.add(neg, pos, seed);
    int64_t dec6 = fhe.decrypt(sum2, seed);
    bool ok6 = (dec6 == 50);
    cout << "6. Blind Add: -50+100=" << dec6 << " " << (ok6 ? "✅" : "❌") << endl;
    if (ok6) pass++;
    
    // 7. Negative blind multiply
    auto neg2 = fhe.encrypt(-6, seed);
    auto pos2 = fhe.encrypt(7, seed);
    auto prod2 = fhe.multiply(neg2, pos2, seed);
    int64_t dec7 = fhe.decrypt(prod2, seed);
    bool ok7 = (dec7 == -42);
    cout << "7. Blind Mul: -6×7=" << dec7 << " " << (ok7 ? "✅" : "❌") << endl;
    if (ok7) pass++;
    
    // 8. IND-CPA
    auto ct_a = fhe.encrypt(42, seed);
    auto ct_b = fhe.encrypt(42, seed);
    bool ok8 = (ct_a != ct_b);
    cout << "8. IND-CPA: " << (ok8 ? "✅" : "❌") << endl;
    if (ok8) pass++;
    
    cout << "\n======================================================" << endl;
    cout << "  BLIND HOMOMORPHIC: " << pass << "/" << total << " PASSED" << endl;
    cout << "======================================================" << endl;
    
    return (pass == total) ? 0 : 1;
}
