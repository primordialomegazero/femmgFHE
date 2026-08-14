#include "../src/fhe/golden_bootstrapping.h"
#include <iostream>

// I-extract ang noise value
long get_noise(const GoldenFHE::Cipher& ct, const GoldenFHE::SecretKey& sk) {
    NTL::ZZ_pX s = sk.sk;
    NTL::ZZ_pX s2 = s * s;
    NTL::ZZ_pX noise = ct.c0 + ct.c1 * s + ct.c2 * s2;
    return NTL::conv<long>(NTL::coeff(noise, 0));
}

int main() {
    GoldenFHE::init_ring();
    
    GoldenFHE::PublicKey pk;
    GoldenFHE::SecretKey sk;
    GoldenFHE::keygen(pk, sk, 42);
    
    long golden_plain = static_cast<long>(GoldenFHE::Q / GoldenFHE::PHI);
    long threshold = static_cast<long>(GoldenFHE::Q / (2 * GoldenFHE::PHI));
    
    std::cout << "Golden plain: " << golden_plain << "\n";
    std::cout << "Threshold: " << threshold << "\n\n";
    
    // Encrypt 1
    GoldenFHE::Cipher ct = GoldenFHE::encrypt(pk, true, 70000000);
    
    std::cout << "Initial noise: " << get_noise(ct, sk) << "\n";
    std::cout << "Decrypt: " << GoldenFHE::decrypt(ct, sk) << "\n\n";
    
    // I-add ng noise (subukan natin 10x muna)
    for (int i = 0; i < 10; i++) {
        ct.c0 = ct.c0 + ct.c0;
        long noise = get_noise(ct, sk);
        
        if ((i + 1) % 5 == 0) {
            std::cout << "After " << (i + 1) << "x noise: " << noise 
                      << " decrypt=" << GoldenFHE::decrypt(ct, sk) << "\n";
        }
    }
    
    std::cout << "\nFinal noise: " << get_noise(ct, sk) << "\n";
    std::cout << "Final decrypt: " << GoldenFHE::decrypt(ct, sk) << "\n";
    
    return 0;
}
