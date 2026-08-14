#include "../src/fhe/golden_bootstrapping.h"
#include <iostream>

int main() {
    GoldenFHE::init_ring();
    
    GoldenFHE::PublicKey pk;
    GoldenFHE::SecretKey sk;
    GoldenFHE::keygen(pk, sk, 42);
    
    GoldenBootstrapping::UnlimitedFHE fhe(pk, sk);
    
    long golden_plain = static_cast<long>(GoldenFHE::Q / GoldenFHE::PHI);
    
    // Test (0,0,0)
    GoldenFHE::Cipher a = GoldenFHE::encrypt(pk, false, 1000);
    GoldenFHE::Cipher b = GoldenFHE::encrypt(pk, false, 2000);
    GoldenFHE::Cipher cin = GoldenFHE::encrypt(pk, false, 3000);
    
    // a XOR b = 0
    GoldenFHE::Cipher a_xor_b = fhe.xor_with_bootstrap(a, b);
    NTL::ZZ_pX noise1 = a_xor_b.c0 + a_xor_b.c1 * sk.sk;
    long v1 = NTL::conv<long>(NTL::coeff(noise1, 0));
    std::cout << "a XOR b: " << v1 << " (expect 0)\n";
    
    // a AND b = 0
    GoldenFHE::Cipher a_and_b = fhe.and_with_bootstrap(a, b);
    NTL::ZZ_pX noise2 = a_and_b.c0 + a_and_b.c1 * sk.sk;
    long v2 = NTL::conv<long>(NTL::coeff(noise2, 0));
    std::cout << "a AND b: " << v2 << " (expect 0)\n";
    
    // cin AND (a XOR b) = 0 AND 0 = 0
    GoldenFHE::Cipher cin_and_axorb = fhe.and_with_bootstrap(cin, a_xor_b);
    NTL::ZZ_pX noise3 = cin_and_axorb.c0 + cin_and_axorb.c1 * sk.sk;
    long v3 = NTL::conv<long>(NTL::coeff(noise3, 0));
    std::cout << "cin AND (a XOR b): " << v3 << " (expect 0)\n";
    
    // cout = (a AND b) OR (cin AND (a XOR b)) = 0 OR 0 = 0
    GoldenFHE::Cipher cout = fhe.or_with_bootstrap(a_and_b, cin_and_axorb);
    NTL::ZZ_pX noise4 = cout.c0 + cout.c1 * sk.sk;
    long v4 = NTL::conv<long>(NTL::coeff(noise4, 0));
    std::cout << "cout: " << v4 << " (expect 0)\n";
    
    bool cout_dec = GoldenFHE::decrypt(cout, sk);
    std::cout << "cout decrypt: " << cout_dec << " (expect 0)\n";
    
    return 0;
}
