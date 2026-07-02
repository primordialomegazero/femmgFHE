#include "../src/core/femmg_operations.h"
#include <iostream>
using namespace std;

int main() {
    FEmmgFHE fhe;
    
    cout << "=== HOMOMORPHIC WITH ENCRYPTED VALUE_INT ===" << endl;
    
    auto a = fhe.encrypt(30);
    auto b = fhe.encrypt(12);
    
    cout << "a.value_int / FP_SCALE = " << a.value_int / phi_constants::FP_SCALE << " (should NOT be 30)" << endl;
    cout << "b.value_int / FP_SCALE = " << b.value_int / phi_constants::FP_SCALE << " (should NOT be 12)" << endl;
    
    auto add_ct = fhe.add(a, b);
    int64_t add_dec = fhe.decrypt(add_ct);
    cout << "\n30 + 12 = " << add_dec << (add_dec == 42 ? " ✅" : " ❌") << endl;
    cout << "add_ct.value_int / FP_SCALE = " << add_ct.value_int / phi_constants::FP_SCALE << " (should NOT be 42)" << endl;
    
    auto c = fhe.encrypt(6);
    auto d = fhe.encrypt(7);
    auto mul_ct = fhe.multiply(c, d);
    int64_t mul_dec = fhe.decrypt(mul_ct);
    cout << "\n6 * 7 = " << mul_dec << (mul_dec == 42 ? " ✅" : " ❌") << endl;
    cout << "mul_ct.value_int / FP_SCALE = " << mul_ct.value_int / phi_constants::FP_SCALE << " (should NOT be 42)" << endl;
    
    // 100 chain test
    auto chain = fhe.encrypt(0);
    auto one = fhe.encrypt(1);
    for (int i = 0; i < 100; i++) chain = fhe.add(chain, one);
    int64_t chain_dec = fhe.decrypt(chain);
    cout << "\n100-chain add = " << chain_dec << (chain_dec == 100 ? " ✅ UNLIMITED DEPTH" : " ❌") << endl;
    
    return (add_dec == 42 && mul_dec == 42 && chain_dec == 100) ? 0 : 1;
}
