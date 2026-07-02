#include "../src/core/femmg_operations.h"
#include <iostream>
using namespace std;

int main() {
    FEmmgFHE fhe;
    
    // Manual trace: depth 1 (should work)
    auto ct1 = fhe.encrypt_fractal(42, 1);
    cout << "Depth 1 decrypt: " << fhe.decrypt_fractal(ct1, 1) << " (expect 42)" << endl;
    cout << "Depth 1 value_int/FP_SCALE = " << ct1.value_int / phi_constants::FP_SCALE << endl;
    
    // Manual trace: depth 2
    auto inner = fhe.encrypt(42);  // encrypt returns encrypted value_int
    cout << "\nInner value_int = " << inner.value_int << endl;
    cout << "Inner value_int/FP_SCALE = " << inner.value_int / phi_constants::FP_SCALE << endl;
    int64_t inner_dec = fhe.decrypt(inner);
    cout << "Inner decrypt = " << inner_dec << endl;
    
    // Now encrypt inner.value_int
    auto outer = fhe.encrypt(inner.value_int / phi_constants::FP_SCALE);
    cout << "Outer value_int/FP_SCALE = " << outer.value_int / phi_constants::FP_SCALE << endl;
    cout << "Outer decrypt = " << fhe.decrypt(outer) << endl;
    
    return 0;
}
