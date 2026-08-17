// Q-EMBED FHE TEST
#include "../src/fhe/golden_fhe_qembed.h"
#include <iostream>

int main() {
    std::cout << "Q-EMBED FHE TEST\n";
    std::cout << "================\n\n";
    
    NTL::ZZ Q = NTL::to_ZZ("115792089237316195423570985008687907853269984665640564039457584007913129640731");
    golden_fhe_qembed::QEmbedFHE fhe(Q, 42);
    
    std::cout << "Public key s = φ^42 = " << fhe.public_s << "\n\n";
    
    // Encryption test
    auto ct0 = fhe.encrypt(false);
    auto ct1 = fhe.encrypt(true);
    
    std::cout << "Encrypt(0) → decrypt: " << fhe.decrypt(ct0) << " (exp 0)\n";
    std::cout << "Encrypt(1) → decrypt: " << fhe.decrypt(ct1) << " (exp 1)\n\n";
    
    // Fresh encryption check
    auto ct1_b = fhe.encrypt(true);
    bool fresh = (ct1 != ct1_b);
    std::cout << "Fresh encryption (different ciphertexts): " << (fresh ? "YES ✓" : "NO ✗") << "\n\n";
    
    // NAND gates
    std::cout << "NAND gates:\n";
    std::cout << "  NAND(0,0) = " << fhe.decrypt(fhe.nand(ct0, ct0)) << " (exp 1)\n";
    std::cout << "  NAND(0,1) = " << fhe.decrypt(fhe.nand(ct0, ct1)) << " (exp 1)\n";
    std::cout << "  NAND(1,0) = " << fhe.decrypt(fhe.nand(ct1, ct0)) << " (exp 1)\n";
    std::cout << "  NAND(1,1) = " << fhe.decrypt(fhe.nand(ct1, ct1)) << " (exp 0)\n\n";
    
    // XOR, AND, OR
    std::cout << "Derived gates:\n";
    std::cout << "  XOR(0,1) = " << fhe.decrypt(fhe.xor_gate(ct0, ct1)) << " (exp 1)\n";
    std::cout << "  AND(1,1) = " << fhe.decrypt(fhe.and_gate(ct1, ct1)) << " (exp 1)\n";
    std::cout << "  OR(0,0) = " << fhe.decrypt(fhe.or_gate(ct0, ct0)) << " (exp 0)\n\n";
    
    // Deep chain
    std::cout << "Deep NAND chain (1000 depths):\n";
    auto current = ct1;
    int errors = 0;
    for (int i = 0; i <= 1000; i++) {
        bool dec = fhe.decrypt(current);
        bool expected = (i % 2 == 0) ? 1 : 0;
        if (dec != expected) errors++;
        current = fhe.nand(current, current);
    }
    std::cout << "Errors: " << errors << "/1001\n";
    
    return 0;
}
