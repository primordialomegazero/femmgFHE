#include "../src/core/femmg_operations.h"
#include <iostream>
using namespace std;

int main() {
    cout << "=== SAME ENGINE vs DIFFERENT ENGINES ===" << endl;
    
    // Test 1: Same engine
    FEmmgFHE fhe1;
    auto a1 = fhe1.encrypt(100);
    auto b1 = fhe1.encrypt(200);
    auto sum1 = fhe1.add(a1, b1);
    int64_t dec1 = fhe1.decrypt(sum1);
    cout << "SAME engine add: " << dec1 << " (expect 300) " << (dec1 == 300 ? "OK" : "FAIL") << endl;
    
    // Test 2: Different engines
    FEmmgFHE fhe2;
    FEmmgFHE fhe3;
    auto a2 = fhe2.encrypt(100);
    auto b2 = fhe3.encrypt(200);
    // Try fhe2.add with ct from fhe3
    auto sum2 = fhe2.add(a2, b2);
    int64_t dec2 = fhe2.decrypt(sum2);
    cout << "DIFF engine add: " << dec2 << " (expect 300) " << (dec2 == 300 ? "OK" : "FAIL") << endl;
    
    // Test 3: Same engine, multiple encrypts
    auto a3 = fhe1.encrypt(100);
    auto b3 = fhe1.encrypt(200);
    auto sum3 = fhe1.add(a3, b3);
    int64_t dec3 = fhe1.decrypt(sum3);
    cout << "SAME engine (2nd try): " << dec3 << " (expect 300) " << (dec3 == 300 ? "OK" : "FAIL") << endl;
    
    return 0;
}
