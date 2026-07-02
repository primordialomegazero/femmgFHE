#include "../src/core/femmg_operations.h"
#include <iostream>
using namespace std;

int main() {
    FEmmgFHE fhe1, fhe2;
    
    auto ct = fhe1.encrypt(42);
    int64_t dec = fhe1.decrypt(ct);
    
    cout << "=== BASIC ENCRYPT/DECRYPT ===" << endl;
    cout << "Encrypt(42) -> decrypt: " << dec << (dec == 42 ? " ✅" : " ❌") << endl;
    
    cout << "\n=== VALUE_INT EXPOSURE ===" << endl;
    cout << "value_int = " << ct.value_int << endl;
    cout << "value_int / FP_SCALE = " << ct.value_int / phi_constants::FP_SCALE << endl;
    cout << "Is plaintext visible? " << (ct.value_int / phi_constants::FP_SCALE == 42 ? "❌ YES (FAIL)" : "✅ NO (ENCRYPTED)") << endl;
    
    cout << "\n=== CROSS-INSTANCE ===" << endl;
    int64_t dec2 = fhe2.decrypt(ct);
    cout << "fhe2 decrypt: " << dec2 << (dec2 != 42 ? " ✅ CHAOS WORKING" : " ❌ FAIL") << endl;
    
    return (dec == 42 && ct.value_int / phi_constants::FP_SCALE != 42 && dec2 != 42) ? 0 : 1;
}
