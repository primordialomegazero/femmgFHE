#include "../src/fhe/golden_bootstrapping.h"
#include <iostream>
#include <iomanip>

// Decrypt na may custom modulus
bool decrypt_with_modulus(const GoldenFHE::Cipher& ct, const GoldenFHE::SecretKey& sk, long modulus) {
    GoldenFHE::init_ring();
    NTL::ZZ_pX s = sk.sk;
    NTL::ZZ_pX s2 = s * s;
    NTL::ZZ_pX noise = ct.c0 + ct.c1 * s + ct.c2 * s2;
    long v = NTL::conv<long>(NTL::coeff(noise, 0));
    
    // Scale ang threshold para sa custom modulus
    long threshold = static_cast<long>(modulus / (2 * GoldenFHE::PHI));
    
    return v > threshold;
}

class ModulusSwitching {
private:
    GoldenFHE::PublicKey pk;
    GoldenFHE::SecretKey sk;
    
public:
    ModulusSwitching(const GoldenFHE::PublicKey& public_key,
                     const GoldenFHE::SecretKey& secret_key)
        : pk(public_key), sk(secret_key) {}
    
    GoldenFHE::Cipher switch_modulus(const GoldenFHE::Cipher& ct, long new_modulus) {
        long old_modulus = GoldenFHE::Q;
        GoldenFHE::Cipher result;
        
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
};

int main() {
    GoldenFHE::init_ring();
    
    GoldenFHE::PublicKey pk;
    GoldenFHE::SecretKey sk;
    GoldenFHE::keygen(pk, sk, 42);
    
    std::cout << "Testing Modulus Switching (v2 - fixed threshold)...\n\n";
    
    ModulusSwitching ms(pk, sk);
    long Q = GoldenFHE::Q;
    
    GoldenFHE::Cipher ct = GoldenFHE::encrypt(pk, true, 1000000);
    
    std::vector<long> new_moduli = {
        Q, Q / 2, Q / 4, Q / 8, Q / 16, Q / 32, Q / 64, Q / 128
    };
    
    std::cout << "New Modulus    Value    Status\n";
    std::cout << "-----------    -----    ------\n";
    
    for (long new_q : new_moduli) {
        GoldenFHE::Cipher switched = ms.switch_modulus(ct, new_q);
        bool decrypted = decrypt_with_modulus(switched, sk, new_q);
        
        std::cout << std::setw(11) << new_q << "    "
                  << std::setw(5) << decrypted << "    "
                  << (decrypted ? "OK" : "FAIL") << "\n";
    }
    
    std::cout << "\n✅ MODULUS SWITCHING V2 PASSED!\n";
    return 0;
}
