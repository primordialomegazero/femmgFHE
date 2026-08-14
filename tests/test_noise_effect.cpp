#include "../src/fhe/golden_bootstrapping.h"
#include <iostream>

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
    
    // Fresh input = 1
    GoldenFHE::Cipher fresh = GoldenFHE::encrypt(pk, true, 1000000);
    std::cout << "Fresh noise: " << get_noise(fresh, sk) << "\n";
    
    // Noisy input (10x)
    GoldenFHE::Cipher noisy = fresh;
    for (int i = 0; i < 10; i++) {
        noisy.c0 = noisy.c0 + noisy.c0;
    }
    std::cout << "Noisy noise: " << get_noise(noisy, sk) << "\n";
    std::cout << "Noisy decrypt: " << GoldenFHE::decrypt(noisy, sk) << "\n";
    
    // XOR with expected=1
    GoldenBootstrapping::UnlimitedFHE fhe(pk, sk);
    GoldenFHE::Cipher expected1 = GoldenFHE::encrypt(pk, true, 40000001);
    GoldenFHE::Cipher eq = fhe.xor_with_bootstrap(noisy, expected1);
    
    std::cout << "XOR(noisy, expected1) noise: " << get_noise(eq, sk) << "\n";
    std::cout << "XOR(noisy, expected1) decrypt: " << GoldenFHE::decrypt(eq, sk) << "\n";
    std::cout << "Expected XOR(1,1) = 0\n";
    
    return 0;
}
