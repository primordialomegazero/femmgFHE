#include "../src/fhe/golden_fibonacci_fhe_v6.h"
#include <iostream>

int main() {
    std::cout << "MULTIPLE Q TEST\n\n";
    
    // Test sa iba't ibang Q sizes na ≡ 1 mod 5
    NTL::ZZ Qs[] = {
        NTL::to_ZZ("4294967291"),                      // 32-bit
        NTL::to_ZZ("18446744073709551611"),            // 64-bit
        NTL::to_ZZ("115792089237316195423570985008687907853269984665640564039457584007913129640731") // 256-bit
    };
    
    for (auto& Q : Qs) {
        std::cout << "=== Q: " << Q << " (" << NTL::NumBits(Q) << " bits) ===\n";
        std::cout << "Q mod 5: " << (Q % 5) << "\n";
        
        golden_fhe_v6::FibonacciFHEV6 fhe(Q, 42);
        
        auto ct0 = fhe.encrypt(false);
        auto ct1 = fhe.encrypt(true);
        
        int pass = 0, total = 8;
        pass += (fhe.decrypt(fhe.nand_gate(ct0, ct0)) == 1);
        pass += (fhe.decrypt(fhe.nand_gate(ct1, ct1)) == 0);
        pass += (fhe.decrypt(fhe.not_gate(ct0)) == 1);
        pass += (fhe.decrypt(fhe.not_gate(ct1)) == 0);
        pass += (fhe.decrypt(fhe.and_gate(ct1, ct1)) == 1);
        pass += (fhe.decrypt(fhe.or_gate(ct0, ct0)) == 0);
        pass += (fhe.decrypt(fhe.xor_gate(ct0, ct1)) == 1);
        pass += (fhe.decrypt(fhe.cnot(ct1, ct0)) == 1);
        
        std::cout << "Pass: " << pass << "/" << total << "\n\n";
    }
    
    return 0;
}
