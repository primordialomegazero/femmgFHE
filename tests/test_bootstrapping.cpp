#include "../src/fhe/golden_bootstrapping.h"
#include <iostream>

int main() {
    GoldenFHE::init_ring();
    
    GoldenFHE::PublicKey pk;
    GoldenFHE::SecretKey sk;
    GoldenFHE::keygen(pk, sk, 42);
    
    // Test unlimited depth na may bootstrapping (mas maaga)
    GoldenBootstrapping::UnlimitedFHE unlimited_fhe(pk, sk, 2);
    
    std::cout << "Testing unlimited depth NOT chain (bootstrap every 2)...\n";
    
    // Start with 1
    GoldenFHE::Cipher val = GoldenFHE::encrypt(pk, true, 1000);
    
    for (int i = 0; i < 20; i++) {
        val = unlimited_fhe.nand_with_bootstrap(val, val);
        
        bool dec = GoldenFHE::decrypt(val, sk);
        bool expected = ((i+1) % 2 == 0);
        
        if (i < 10 || i % 5 == 0) {
            std::cout << "Depth " << i+1 << ": " << dec << " (expected " << expected << ")\n";
        }
        
        if (dec != expected) {
            std::cout << "❌ FAILED at depth " << i+1 << "\n";
            return 1;
        }
    }
    
    std::cout << "\n✅ Bootstrapping test passed! 20 levels ng NOT chain!\n";
    
    return 0;
}
