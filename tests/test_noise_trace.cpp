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
        // Bago mag-NOT
        NTL::ZZ_pX noise_before = val.c0 + val.c1 * sk.sk;
        long v_before = NTL::conv<long>(NTL::coeff(noise_before, 0));
        
        // Apply NOT
        val = GoldenFHE::NOT(val);
        
        // Pagkatapos
        NTL::ZZ_pX noise_after = val.c0 + val.c1 * sk.sk;
        long v_after = NTL::conv<long>(NTL::coeff(noise_after, 0));
        
        std::cout << "Depth " << i+1 << ": before=" << v_before << " after=" << v_after 
                  << " (expected " << ((i+1) % 2 == 0 ? golden_plain : 0) << ")\n";
        
        // I-check ang distance sa threshold
        long expected = ((i+1) % 2 == 0) ? golden_plain : 0;
        long distance = std::abs(v_after - expected);
        std::cout << "  Distance from expected: " << distance << "\n";
    }
    
    return 0;
}
