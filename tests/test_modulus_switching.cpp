#include "../src/fhe/golden_bootstrapping.h"
#include <iostream>
#include <iomanip>

// Modulus Switching: I-scale ang ciphertext sa mas maliit na modulus
// Para sa noise reduction sa malalim na circuits

class ModulusSwitching {
private:
    GoldenFHE::PublicKey pk;
    GoldenFHE::SecretKey sk;
    
public:
    ModulusSwitching(const GoldenFHE::PublicKey& public_key,
                     const GoldenFHE::SecretKey& secret_key)
        : pk(public_key), sk(secret_key) {}
    
    // Switch mula sa Q papunta sa mas maliit na modulus q
    GoldenFHE::Cipher switch_modulus(const GoldenFHE::Cipher& ct, long new_modulus) {
        long old_modulus = GoldenFHE::Q;
        
        GoldenFHE::Cipher result;
        
        // Scale: round(c * new_q / old_q)
        for (long i = 0; i <= NTL::deg(ct.c0); i++) {
            long old_coeff = NTL::conv<long>(NTL::coeff(ct.c0, i));
            long new_coeff = (old_coeff * new_modulus + old_modulus / 2) / old_modulus;
            NTL::SetCoeff(result.c0, i, new_coeff % new_modulus);
        }
        
        for (long i = 0; i <= NTL::deg(ct.c1); i++) {
            long old_coeff = NTL::conv<long>(NTL::coeff(ct.c1, i));
            long new_coeff = (old_coeff * new_modulus + old_modulus / 2) / old_modulus;
            NTL::SetCoeff(result.c1, i, new_coeff % new_modulus);
        }
        
        result.c2 = NTL::ZZ_pX();
        
        return result;
    }
    
    // I-check kung ang modulus switching ay nagpo-preserve ng decryption
    bool test_modulus_switch(const GoldenFHE::Cipher& ct, long new_modulus) {
        bool before = GoldenFHE::decrypt(ct, sk);
        
        // I-switch at i-decrypt (sa simpleng version, i-check natin ang scaling)
        GoldenFHE::Cipher switched = switch_modulus(ct, new_modulus);
        
        // I-decrypt ang switched (gamit ang orihinal na modulus para sa test)
        bool after = GoldenFHE::decrypt(switched, sk);
        
        return before == after;
    }
};

int main() {
    GoldenFHE::init_ring();
    
    GoldenFHE::PublicKey pk;
    GoldenFHE::SecretKey sk;
    GoldenFHE::keygen(pk, sk, 42);
    
    std::cout << "Testing Modulus Switching...\n\n";
    
    ModulusSwitching ms(pk, sk);
    
    long Q = GoldenFHE::Q;
    std::cout << "Original modulus Q: " << Q << "\n\n";
    
    GoldenFHE::Cipher ct = GoldenFHE::encrypt(pk, true, 1000000);
    
    std::vector<long> new_moduli = {
        Q, Q / 2, Q / 4, Q / 8, Q / 16, Q / 32, Q / 64, Q / 128
    };
    
    std::cout << "New Modulus    Value    Status\n";
    std::cout << "-----------    -----    ------\n";
    
    for (long new_q : new_moduli) {
        GoldenFHE::Cipher switched = ms.switch_modulus(ct, new_q);
        bool decrypted = GoldenFHE::decrypt(switched, sk);
        
        std::cout << std::setw(11) << new_q << "    "
                  << std::setw(5) << decrypted << "    "
                  << (decrypted ? "OK" : "FAIL") << "\n";
    }
    
    std::cout << "\n✅ MODULUS SWITCHING TEST COMPLETE!\n";
    return 0;
}
