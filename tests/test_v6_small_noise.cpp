// V6 with SMALL NOISE — Test kung kaya ng mas malalim na depth
#include "../src/fhe/golden_fibonacci_fhe_v6.h"
#include <iostream>

int main() {
    std::cout << "V6 SMALL NOISE TEST\n";
    std::cout << "===================\n\n";
    
    NTL::ZZ Q = NTL::to_ZZ("115792089237316195423570985008687907853269984665640564039457584007913129640731");
    golden_fhe_v6::FibonacciFHEV6 fhe(Q, 42);
    
    auto ct1 = fhe.encrypt(true);
    auto current = ct1;
    
    std::cout << "Deep NAND chain (100 depths):\n";
    int errors = 0;
    for (int i = 0; i <= 100; i++) {
        bool result = fhe.decrypt(current);
        bool expected = (i % 2 == 0) ? 1 : 0;
        if (result != expected) {
            if (errors < 5) {
                std::cout << "  Depth " << i << ": got " << result << ", exp " << expected << " ✗\n";
            }
            errors++;
        }
        current = fhe.nand_gate(current, current);
    }
    
    std::cout << "\nTotal errors: " << errors << "/101\n";
    return 0;
}
