#include "../src/core/femmg_operations.h"
#include <iostream>
using namespace std;

int main() {
    FEmmgFHE fhe;
    
    auto a = fhe.encrypt(30);
    auto b = fhe.encrypt(12);
    
    // Check if integrity tag exists
    cout << "a.integrity_tag = 0x" << hex << a.integrity_tag << dec << endl;
    cout << "b.integrity_tag = 0x" << hex << b.integrity_tag << dec << endl;
    
    auto sum = fhe.add(a, b);
    cout << "sum.integrity_tag = 0x" << hex << sum.integrity_tag << dec << endl;
    
    // Test tamper detection
    auto ct = fhe.encrypt(42);
    ct.chaos_history[0] = 999999.0;  // Tamper
    int64_t dec = fhe.decrypt(ct);
    cout << "\nTampered decrypt: " << dec << (dec != 42 ? " ✅ DETECTED" : " ❌ FAIL") << endl;
    
    // Test tamper value_int
    auto ct2 = fhe.encrypt(42);
    ct2.value_int ^= 1;  // Flip 1 bit
    int64_t dec2 = fhe.decrypt(ct2);
    cout << "Flipped value_int decrypt: " << dec2 << (dec2 != 42 ? " ✅ DETECTED" : " ❌ FAIL") << endl;
    
    return 0;
}
