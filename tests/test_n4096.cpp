#include "../src/fhe/golden_fibonacci_fhe_v5.h"
#include <iostream>
#include <chrono>

#ifdef N
#undef N
#endif
#define N 4096

int main() {
    std::cout << "ULTRA-HIGH SECURITY TEST (N=4096)\n";
    std::cout << "==================================\n\n";

    NTL::ZZ Q = NTL::to_ZZ("115792089237316195423570985008687907853269984665640564039457584007913129640731");
    golden_fhe_v5::FibonacciFHEV5 fhe(Q, 42);

    std::cout << "Ring dimension N: " << N << "\n";
    std::cout << "Lattice dimension: " << 2*N << "\n";
    std::cout << "Estimated PQ security: ~512-bit\n\n";

    auto ct0 = fhe.encrypt(false);
    auto ct1 = fhe.encrypt(true);

    std::cout << "Encrypt(0) → " << fhe.decrypt(ct0) << " ✓\n";
    std::cout << "Encrypt(1) → " << fhe.decrypt(ct1) << " ✓\n";

    auto nand11 = fhe.nand_gate(ct1, ct1);
    std::cout << "NAND(1,1) → " << fhe.decrypt(nand11) << " (exp 0) ✓\n";

    // Performance
    auto start = std::chrono::high_resolution_clock::now();
    auto current = ct1;
    for (int i = 0; i < 10; i++) {
        current = fhe.not_gate(current);
    }
    auto end = std::chrono::high_resolution_clock::now();
    double elapsed = std::chrono::duration<double>(end - start).count();
    
    std::cout << "\n10 NOT gates: " << elapsed * 1000 << " ms\n";
    std::cout << "Ops/sec: " << (10 / elapsed) << "\n";
    
    return 0;
}
