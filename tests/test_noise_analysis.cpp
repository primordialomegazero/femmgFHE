#include "../src/fhe/golden_quantum_fhe.h"
#include <iostream>

int main() {
    GoldenFHE::init_ring();
    
    GoldenFHE::PublicKey pk;
    GoldenFHE::SecretKey sk;
    GoldenFHE::keygen(pk, sk, 42);
    
    long golden_plain = static_cast<long>(GoldenFHE::Q / GoldenFHE::PHI);
    
    // I-test ang NAND(1,1) sa iba't ibang nonce values
    for (uint64_t nonce = 1000000; nonce < 1000100; nonce += 10) {
        GoldenFHE::Cipher one1 = GoldenFHE::encrypt(pk, true, nonce);
        GoldenFHE::Cipher one2 = GoldenFHE::encrypt(pk, true, nonce + 1);
        
        GoldenFHE::Cipher nand11 = GoldenFHE::nand_gate(one1, one2);
        
        NTL::ZZ_pX noise = nand11.c0 + nand11.c1 * sk.sk;
        long v = NTL::conv<long>(NTL::coeff(noise, 0));
        
        if (v > GoldenFHE::Q / 2) {
            std::cout << "nonce=" << nonce << " -> WRONG (v=" << v << ")\n";
        }
    }
    
    std::cout << "Done scanning.\n";
    return 0;
}
