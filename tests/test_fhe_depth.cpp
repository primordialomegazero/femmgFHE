#include "../src/fhe/golden_quantum_fhe.h"
#include <iostream>

int main() {
    GoldenFHE::init_ring();
    
    GoldenFHE::PublicKey pk;
    GoldenFHE::SecretKey sk;
    GoldenFHE::keygen(pk, sk, 42);
    
    long golden_plain = static_cast<long>(GoldenFHE::Q / GoldenFHE::PHI);
    long threshold = static_cast<long>(GoldenFHE::Q / (2 * GoldenFHE::PHI));
    
    std::cout << "Golden plain: " << golden_plain << "\n";
    std::cout << "Threshold: " << threshold << "\n\n";
    
    // Start with 1
    GoldenFHE::Cipher val = GoldenFHE::encrypt(pk, true, 1000);
    
    for (int i = 0; i < 5; i++) {
        val = GoldenFHE::NOT(val);
        
        NTL::ZZ_pX noise = val.c0 + val.c1 * sk.sk;
        long v = NTL::conv<long>(NTL::coeff(noise, 0));
        
        bool dec = v > threshold;
        bool expected = ((i+1) % 2 == 0);
        
        std::cout << "Depth " << i+1 << ": value=" << v 
                  << " dec=" << dec << " expected=" << expected << "\n";
    }
    
    return 0;
}
