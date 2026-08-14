#include "../src/fhe/golden_relinearization.h"
#include <iostream>

int main() {
    GoldenFHE::init_ring();
    
    GoldenFHE::PublicKey pk;
    GoldenFHE::SecretKey sk;
    GoldenFHE::keygen(pk, sk, 42);
    
    std::cout << "Testing Relinearization...\n\n";
    
    // Generate relinearization key
    GoldenRelinearization::RelinearizationKey rlk;
    rlk.generate(pk, sk, 7777777);
    
    // Test: multiply at relinearize
    GoldenFHE::Cipher a = GoldenFHE::encrypt(pk, true, 1000000);
    GoldenFHE::Cipher b = GoldenFHE::encrypt(pk, true, 1000001);
    
    // Multiply (3 components)
    GoldenFHE::Cipher product;
    product.c0 = a.c0 * b.c0;
    product.c1 = a.c0 * b.c1 + a.c1 * b.c0;
    product.c2 = a.c1 * b.c1;
    
    // Decrypt bago relinearization
    bool before = GoldenFHE::decrypt(product, sk);
    std::cout << "Before relinearization: " << before << "\n";
    
    // Relinearize
    GoldenFHE::Cipher relinearized = GoldenRelinearization::relinearize(product, rlk);
    
    // Decrypt pagkatapos
    bool after = GoldenFHE::decrypt(relinearized, sk);
    std::cout << "After relinearization: " << after << "\n";
    
    // I-check kung ang c2 ay zero na
    bool c2_zero = true;
    for (long i = 0; i <= NTL::deg(relinearized.c2); i++) {
        if (NTL::conv<long>(NTL::coeff(relinearized.c2, i)) != 0) {
            c2_zero = false;
            break;
        }
    }
    std::cout << "c2 is zero: " << (c2_zero ? "YES" : "NO") << "\n";
    
    if (before == after && c2_zero) {
        std::cout << "\n✅ RELINEARIZATION TEST PASSED!\n";
        return 0;
    }
    
    std::cout << "\n❌ FAILED\n";
    return 1;
}
