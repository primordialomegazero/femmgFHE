#include "src/femmg_fhe.h"
#include "phi_parallel_kem.h"
#include <iostream>
int main() {
    // Test FHE
    FEmmgFHE fhe;
    auto ct = fhe.encrypt(42);
    int64_t m = fhe.decrypt(ct);
    std::cout << "FHE: 42 → " << m << (m==42?" ✅":" ❌") << std::endl;
    
    // Test KEM
    int kem_ok = phi_parallel::phi_parallel_self_test();
    std::cout << "KEM: self-test " << (kem_ok?"✅":"❌") << std::endl;
    
    return (m==42 && kem_ok) ? 0 : 1;
}
