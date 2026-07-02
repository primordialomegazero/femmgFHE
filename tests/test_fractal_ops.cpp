#include "../src/core/femmg_operations.h"
#include <iostream>
using namespace std;

int main() {
    FEmmgFHE fhe;
    
    // Fractal add
    auto a = fhe.encrypt_fractal(30, 3);
    auto b = fhe.encrypt_fractal(12, 3);
    auto add = fhe.add_fractal(a, b, 3);
    int64_t add_dec = fhe.decrypt_fractal(add, 3);
    cout << "Fractal Add (depth 3): 30+12=" << add_dec << (add_dec == 42 ? " ✅" : " ❌") << endl;
    cout << "  value_int/FP_SCALE = " << add.value_int / phi_constants::FP_SCALE << " (NOT 42)" << endl;
    
    // Fractal mul
    auto c = fhe.encrypt_fractal(6, 3);
    auto d = fhe.encrypt_fractal(7, 3);
    auto mul = fhe.multiply_fractal(c, d, 3);
    int64_t mul_dec = fhe.decrypt_fractal(mul, 3);
    cout << "Fractal Mul (depth 3): 6*7=" << mul_dec << (mul_dec == 42 ? " ✅" : " ❌") << endl;
    cout << "  value_int/FP_SCALE = " << mul.value_int / phi_constants::FP_SCALE << " (NOT 42)" << endl;
    
    return (add_dec == 42 && mul_dec == 42) ? 0 : 1;
}
