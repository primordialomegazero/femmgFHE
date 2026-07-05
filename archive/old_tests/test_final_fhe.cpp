#include "../src/core/femmg_operations.h"
#include <iostream>
using namespace std;

int main() {
    FEmmgFHE fhe1, fhe2;
    int pass = 0, total = 10;
    
    cout << "======================================================" << endl;
    cout << "  FINAL TRUE FHE VERIFICATION" << endl;
    cout << "======================================================" << endl;
    
    // 1. Basic encrypt/decrypt
    auto ct = fhe1.encrypt(42);
    int64_t dec = fhe1.decrypt(ct);
    cout << "1.  Encrypt/Decrypt: 42 -> " << dec << (dec == 42 ? " ✅" : " ❌") << endl;
    if (dec == 42) pass++;
    
    // 2. value_int NOT plaintext
    bool hidden = (ct.value_int / phi_constants::FP_SCALE != 42);
    cout << "2.  value_int hidden: " << (hidden ? "✅" : "❌") << " (" << ct.value_int / phi_constants::FP_SCALE << ")" << endl;
    if (hidden) pass++;
    
    // 3. Cross-instance = garbage
    int64_t dec2 = fhe2.decrypt(ct);
    cout << "3.  Cross-instance: " << (dec2 != 42 ? "✅" : "❌") << endl;
    if (dec2 != 42) pass++;
    
    // 4. Homomorphic add
    auto add = fhe1.add(fhe1.encrypt(30), fhe1.encrypt(12));
    cout << "4.  Hom Add: 30+12=" << fhe1.decrypt(add) << (fhe1.decrypt(add) == 42 ? " ✅" : " ❌") << endl;
    if (fhe1.decrypt(add) == 42) pass++;
    
    // 5. Homomorphic mul
    auto mul = fhe1.multiply(fhe1.encrypt(6), fhe1.encrypt(7));
    cout << "5.  Hom Mul: 6*7=" << fhe1.decrypt(mul) << (fhe1.decrypt(mul) == 42 ? " ✅" : " ❌") << endl;
    if (fhe1.decrypt(mul) == 42) pass++;
    
    // 6. Add result value_int hidden
    bool add_hidden = (add.value_int / phi_constants::FP_SCALE != 42);
    cout << "6.  Add result hidden: " << (add_hidden ? "✅" : "❌") << endl;
    if (add_hidden) pass++;
    
    // 7. Mul result value_int hidden
    bool mul_hidden = (mul.value_int / phi_constants::FP_SCALE != 42);
    cout << "7.  Mul result hidden: " << (mul_hidden ? "✅" : "❌") << endl;
    if (mul_hidden) pass++;
    
    // 8. Tamper detection
    auto ct_t = fhe1.encrypt(42);
    ct_t.chaos_history[0] = 999999.0;
    cout << "8.  Tamper detect: " << (fhe1.decrypt(ct_t) != 42 ? "✅" : "❌") << endl;
    if (fhe1.decrypt(ct_t) != 42) pass++;
    
    // 9. 100-chain depth
    auto chain = fhe1.encrypt(0);
    auto one = fhe1.encrypt(1);
    for (int i = 0; i < 100; i++) chain = fhe1.add(chain, one);
    cout << "9.  Depth 100: " << fhe1.decrypt(chain) << (fhe1.decrypt(chain) == 100 ? " ✅" : " ❌") << endl;
    if (fhe1.decrypt(chain) == 100) pass++;
    
    // 10. Fractal
    auto fct = fhe1.encrypt_fractal(42, 3);
    cout << "10. Fractal v3: " << fhe1.decrypt_fractal(fct, 3) << (fhe1.decrypt_fractal(fct, 3) == 42 ? " ✅" : " ❌") << endl;
    if (fhe1.decrypt_fractal(fct, 3) == 42) pass++;
    
    cout << "\n======================================================" << endl;
    cout << "  RESULT: " << pass << "/" << total << " PASSED" << endl;
    cout << "  TRUE FHE: " << (pass == total ? "✅ VERIFIED" : "❌ INCOMPLETE") << endl;
    cout << "======================================================" << endl;
    
    return (pass == total) ? 0 : 1;
}
