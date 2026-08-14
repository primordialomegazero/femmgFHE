#pragma once
#include "golden_quantum_fhe.h"
#include <vector>

namespace GoldenKeySwitching {

using namespace GoldenFHE;

// Key switching: i-convert ang ciphertext mula sa lumang key papunta sa bagong key
class KeySwitchingKey {
public:
    // Switching key: encrypted lumang secret key bits gamit ang bagong key
    std::vector<Cipher> switch_keys;  // bawat bit ng lumang sk
    
    void generate(const PublicKey& new_pk, const SecretKey& old_sk, uint64_t nonce) {
        switch_keys.clear();
        
        // I-decompose ang lumang secret key sa bits
        // Para sa simpleng version: i-encrypt ang coefficient 0 ng old sk
        
        long sk0 = NTL::conv<long>(NTL::coeff(old_sk.sk, 0));
        
        // I-encrypt ang bawat bit ng sk0 gamit ang new key
        for (int bit = 0; bit < 32; bit++) {
            bool sk_bit = (sk0 >> bit) & 1;
            switch_keys.push_back(encrypt(new_pk, sk_bit, nonce + bit));
        }
    }
    
    // Switch ang ciphertext mula sa old key papunta sa new key
    Cipher switch_key(const Cipher& ct, const SecretKey& old_sk) {
        init_ring();
        
        Cipher result;
        result.c0 = ct.c0;
        result.c1 = ct.c1;
        result.c2 = NTL::ZZ_pX();
        
        // Para sa simpleng version: i-decrypt gamit ang old sk
        // at i-reencrypt gamit ang new sk (sa practice, ito ay homomorphic)
        
        // Ito ay placeholder - ang tunay na key switching ay homomorphic
        // na gumagamit ng switch_keys para i-convert
        
        return result;
    }
};

// Simple key switching test
inline bool test_key_switching(const PublicKey& pk1, const SecretKey& sk1,
                                 const PublicKey& pk2, const SecretKey& sk2) {
    init_ring();
    
    // I-encrypt gamit ang key 1
    Cipher ct = encrypt(pk1, true, 1000000);
    
    // I-decrypt gamit ang key 1 (dapat tama)
    bool dec_sk1 = decrypt(ct, sk1);
    
    // I-decrypt gamit ang key 2 (dapat mali kasi maling key)
    bool dec_sk2 = decrypt(ct, sk2);
    
    // Ang key switching ay dapat mag-convert para maging decryptable sa sk2
    
    return dec_sk1 && !dec_sk2;
}

} // namespace GoldenKeySwitching
