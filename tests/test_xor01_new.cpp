#include "../src/fhe/golden_bootstrapping.h"
#include <iostream>

int main() {
    GoldenFHE::init_ring();
    
    GoldenFHE::PublicKey pk;
    GoldenFHE::SecretKey sk;
    GoldenFHE::keygen(pk, sk, 42);
    
    GoldenBootstrapping::UnlimitedFHE fhe(pk, sk);
    
    long golden_plain = static_cast<long>(GoldenFHE::Q / GoldenFHE::PHI);
    
    GoldenFHE::Cipher a = GoldenFHE::encrypt(pk, false, 1000);
    GoldenFHE::Cipher b = GoldenFHE::encrypt(pk, true, 2000);
    
    GoldenFHE::Cipher xor_result = fhe.xor_with_bootstrap(a, b);
    
    NTL::ZZ_pX noise = xor_result.c0 + xor_result.c1 * sk.sk;
    long v = NTL::conv<long>(NTL::coeff(noise, 0));
    
    bool dec = GoldenFHE::decrypt(xor_result, sk);
    
    std::cout << "XOR(0,1) value: " << v << "\n";
    std::cout << "XOR(0,1) decrypt: " << dec << " (expected 1)\n";
    std::cout << "Golden plain: " << golden_plain << "\n";
    
    return 0;
}
