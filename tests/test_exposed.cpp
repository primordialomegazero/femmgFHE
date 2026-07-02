#include "../src/core/femmg_operations.h"
#include <iostream>
using namespace std;

int main() {
    FEmmgFHE fhe;
    
    cout << "=== VALUE_INT EXPOSURE TEST ===" << endl;
    
    auto ct_a = fhe.encrypt(42);
    auto ct_b = fhe.encrypt(7);
    
    cout << "ct_a.value_int = " << ct_a.value_int << endl;
    cout << "ct_a.value_int / FP_SCALE = " << ct_a.value_int / phi_constants::FP_SCALE << endl;
    cout << "ct_b.value_int = " << ct_b.value_int << endl;
    cout << "ct_b.value_int / FP_SCALE = " << ct_b.value_int / phi_constants::FP_SCALE << endl;
    
    auto ct_c = fhe.multiply(ct_a, ct_b);
    cout << "After multiply, ct_c.value_int / FP_SCALE = " 
         << ct_c.value_int / phi_constants::FP_SCALE << endl;
    cout << "Expected: 294" << endl;
    cout << "Decrypted: " << fhe.decrypt(ct_c) << endl;
    
    if (ct_a.value_int / phi_constants::FP_SCALE == 42 &&
        ct_b.value_int / phi_constants::FP_SCALE == 7) {
        cout << "\n🔴 VERDICT: value_int is PLAINTEXT. NOT FHE." << endl;
    } else {
        cout << "\n🟢 VERDICT: value_int is ENCRYPTED. LEGIT FHE!" << endl;
    }
    
    return 0;
}
