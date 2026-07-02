#include "../src/core/femmg_operations.h"
#include <iostream>
using namespace std;

int main() {
    FEmmgFHE fhe;
    
    auto ct = fhe.encrypt_fractal(42, 3);
    int64_t dec = fhe.decrypt_fractal(ct, 3);
    cout << "Fractal(42, depth=3): " << dec << (dec == 42 ? " ✅" : " ❌") << endl;
    cout << "value_int / FP_SCALE = " << ct.value_int / phi_constants::FP_SCALE << " (should NOT be 42)" << endl;
    
    return (dec == 42 && ct.value_int / phi_constants::FP_SCALE != 42) ? 0 : 1;
}
