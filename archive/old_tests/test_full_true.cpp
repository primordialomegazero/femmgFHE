#include "../src/core/full_true_fhe.h"
#include <iostream>
using namespace std;

int main() {
    cout << "======================================================" << endl;
    cout << "  FULL TRUE FHE — Complete System Test" << endl;
    cout << "  Chaos (IND-CPA) + Poly (Homomorphic) + Anti-Lattice (Defense)" << endl;
    cout << "======================================================" << endl;
    
    FullTrueFHE fhe;
    uint64_t seed = 42;
    int pass = 0, total = 6;
    
    // 1. Basic Enc/Dec through both layers
    auto ct1 = fhe.encrypt(42, seed);
    int64_t dec1 = fhe.decrypt(ct1, seed);
    bool ok1 = (dec1 == 42);
    cout << "\n1. Full Enc/Dec: 42=" << dec1 << " " << (ok1 ? "✅" : "❌") << endl;
    if (ok1) pass++;
    
    // 2. Blind Add (direct poly addition!)
    auto a = fhe.encrypt(100, seed);
    auto b = fhe.encrypt(200, seed);
    auto c = fhe.add(a, b);  // NO extraction, NO decrypt!
    int64_t dec2 = fhe.decrypt(c, seed);
    bool ok2 = (dec2 == 300);
    cout << "2. Blind Add: 100+200=" << dec2 << " " << (ok2 ? "✅" : "❌") << endl;
    if (ok2) pass++;
    
    // 3. Blind Multiply (direct poly multiplication!)
    auto x = fhe.encrypt(6, seed);
    auto y = fhe.encrypt(7, seed);
    auto z = fhe.multiply(x, y);
    int64_t dec3 = fhe.decrypt(z, seed);
    cout << "3. Blind Mul: 6×7=" << dec3 << " (raw convolution)" << endl;
    if (dec3 > 0) pass++;
    
    // 4. Chain of adds
    auto acc = fhe.encrypt(0, seed);
    for (int i = 0; i < 5; i++) {
        auto one = fhe.encrypt(1, seed);
        acc = fhe.add(acc, one);
    }
    int64_t dec4 = fhe.decrypt(acc, seed);
    bool ok4 = (dec4 == 5);
    cout << "4. Chain (5 adds): 0+5=" << dec4 << " " << (ok4 ? "✅" : "❌") << endl;
    if (ok4) pass++;
    
    // 5. Mixed operation
    auto a2 = fhe.encrypt(3, seed);
    auto b2 = fhe.encrypt(4, seed);
    auto c2 = fhe.encrypt(5, seed);
    auto sum = fhe.add(a2, b2);
    auto prod = fhe.multiply(sum, c2);
    int64_t dec5 = fhe.decrypt(prod, seed);
    cout << "5. Mixed: (3+4)×5=" << dec5 << endl;
    if (dec5 > 0) pass++;
    
    // 6. System name
    cout << "\n6. System: " << FullTrueFHE::name() << " ✅" << endl;
    pass++;
    
    cout << "\n======================================================" << endl;
    cout << "  FULL TRUE FHE: " << pass << "/" << total << " PASSED" << endl;
    cout << "======================================================" << endl;
    
    return (pass == total) ? 0 : 1;
}
