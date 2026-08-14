#pragma once
#include "golden_quantum_fhe.h"
#include <vector>

namespace GoldenRelinearization {

using namespace GoldenFHE;

// Relinearization key: encrypted s^2 para sa key switching
class RelinearizationKey {
public:
    Cipher rlk_c0;  // Encrypted s^2 (c0 part)
    Cipher rlk_c1;  // Encrypted s^2 (c1 part)
    
    RelinearizationKey() = default;
    
    // Generate relinearization key mula sa secret key
    void generate(const PublicKey& pk, const SecretKey& sk, uint64_t nonce) {
        // I-encrypt ang s^2 bilang ciphertext
        NTL::ZZ_pX s2 = sk.sk * sk.sk;
        
        // I-convert s2 sa bits at i-encrypt
        // Para sa simpleng version: i-encrypt ang buong s2 polynomial
        
        // c0 = -(a*s + e) + s2 (message ay s2)
        // c1 = a
        
        uint64_t state = nonce;
        NTL::ZZ_pX u, e0, e1;
        for (int i = 0; i < N; i++) {
            state ^= (state << 13);
            state ^= (state >> 7);
            state ^= (state << 17);
            NTL::SetCoeff(u, i, (state % 3) - 1);
            NTL::SetCoeff(e0, i, (state % 10000) == 0 ? 1 : 0);
            NTL::SetCoeff(e1, i, (state % 10000) == 0 ? 1 : 0);
        }
        
        rlk_c0.c0 = pk.pk0 * u + e0 + s2;
        rlk_c0.c1 = pk.pk1 * u + e1;
        rlk_c0.c2 = NTL::ZZ_pX();
        
        rlk_c1.c0 = NTL::ZZ_pX();
        rlk_c1.c1 = NTL::ZZ_pX();
        rlk_c1.c2 = NTL::ZZ_pX();
    }
};

// Relinearization: i-reduce ang (c0, c1, c2) sa (c0, c1)
// Gamit ang relinearization key
inline Cipher relinearize(const Cipher& ct, const RelinearizationKey& rlk) {
    init_ring();
    
    // Ang c2 term ay may s^2, kaya:
    // new_c0 = c0 + c2 * rlk_c0
    // new_c1 = c1 + c2 * rlk_c1
    
    Cipher result;
    
    // c2 * rlk (homomorphic multiplication with relinearization key)
    NTL::ZZ_pX t0 = ct.c2 * rlk.rlk_c0.c0;
    NTL::ZZ_pX t1 = ct.c2 * rlk.rlk_c0.c1 + ct.c2 * rlk.rlk_c0.c2;
    
    result.c0 = ct.c0 + t0;
    result.c1 = ct.c1 + t1;
    result.c2 = NTL::ZZ_pX();  // Zero out c2
    
    return result;
}

// Test: relinearization preserves correctness
inline bool test_relinearization(const PublicKey& pk, const SecretKey& sk) {
    init_ring();
    
    RelinearizationKey rlk;
    rlk.generate(pk, sk, 7777777);
    
    // I-encrypt ang 1 at 1
    Cipher a = encrypt(pk, true, 1000000);
    Cipher b = encrypt(pk, true, 1000001);
    
    // Multiply (magkakaroon ng c2)
    Cipher product;
    product.c0 = a.c0 * b.c0;
    product.c1 = a.c0 * b.c1 + a.c1 * b.c0;
    product.c2 = a.c1 * b.c1;
    
    // I-check bago relinearization
    bool before = decrypt(product, sk);
    
    // Relinearize
    Cipher relinearized = relinearize(product, rlk);
    
    // I-check pagkatapos
    bool after = decrypt(relinearized, sk);
    
    return before == after;
}

} // namespace GoldenRelinearization
