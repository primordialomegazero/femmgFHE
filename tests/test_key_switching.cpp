#include "../src/fhe/golden_key_switching.h"
#include <iostream>

int main() {
    GoldenFHE::init_ring();
    
    // Key pair 1
    GoldenFHE::PublicKey pk1;
    GoldenFHE::SecretKey sk1;
    GoldenFHE::keygen(pk1, sk1, 42);
    
    // Key pair 2
    GoldenFHE::PublicKey pk2;
    GoldenFHE::SecretKey sk2;
    GoldenFHE::keygen(pk2, sk2, 99);
    
    std::cout << "Testing Key Switching...\n\n";
    
    // I-encrypt gamit ang key 1
    GoldenFHE::Cipher ct = GoldenFHE::encrypt(pk1, true, 1000000);
    
    // I-decrypt gamit ang key 1
    bool dec_sk1 = GoldenFHE::decrypt(ct, sk1);
    std::cout << "Decrypt with sk1: " << dec_sk1 << " (expected 1)\n";
    
    // I-decrypt gamit ang key 2 (dapat mali)
    bool dec_sk2 = GoldenFHE::decrypt(ct, sk2);
    std::cout << "Decrypt with sk2 (wrong key): " << dec_sk2 << " (random/mali)\n";
    
    // Generate switching key
    GoldenKeySwitching::KeySwitchingKey ksk;
    ksk.generate(pk2, sk1, 5555555);
    
    std::cout << "\nSwitching key generated (32 bits of sk1 encrypted with pk2)\n";
    std::cout << "Switch keys count: " << ksk.switch_keys.size() << "\n";
    
    // I-verify na ang switching keys ay decryptable sa sk2
    int correct_bits = 0;
    for (int i = 0; i < 32; i++) {
        bool bit = GoldenFHE::decrypt(ksk.switch_keys[i], sk2);
        bool expected = (NTL::conv<long>(NTL::coeff(sk1.sk, 0)) >> i) & 1;
        if (bit == expected) correct_bits++;
    }
    
    std::cout << "Switch keys correctness: " << correct_bits << "/32 bits\n";
    
    if (dec_sk1 && correct_bits == 32) {
        std::cout << "\n✅ KEY SWITCHING TEST PASSED (structure ready)!\n";
        std::cout << "Note: Full homomorphic key switching needs more work.\n";
        return 0;
    }
    
    std::cout << "\n❌ FAILED\n";
    return 1;
}
