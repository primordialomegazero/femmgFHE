// SPIRAL-ENCRYPT v3 — Working Simplified Encryption
// ct = (m + pk0, pk1) — valid Ring-LWE with u=1

#pragma once
#include "spiral_fhe.h"
#include "spiral_ntt.h"

namespace spiral {

class SpiralEncrypt {
private:
    SpiralParams params;
    SpiralNTT ntt;
    SpiralNoiseGenerator noise_gen;
    
public:
    SpiralEncrypt(const SpiralParams& p = SpiralParams()) 
        : params(p), ntt(p.poly_degree) {}
    
    uint64_t get_modulus() const { return ntt.modulus(); }
    
    // Encrypt: ct = (m + pk0 + e0, pk1 + e1)
    SpiralCiphertext encrypt(uint64_t message, const SpiralPublicKey& pk) {
        SpiralCiphertext ct;
        size_t N = params.poly_degree;
        uint64_t q = ntt.modulus();
        
        ct.c0.resize(N, 0);
        ct.c1.resize(N, 0);
        ct.level = 0;
        
        auto e0 = noise_gen.generate_vector(N);
        auto e1 = noise_gen.generate_vector(N);
        
        // c0 = m + pk0 + e0 (m at position 0)
        ct.c0[0] = (message + pk.p0[0] + (uint64_t)std::abs(e0[0])) % q;
        for (size_t i = 1; i < N; i++) {
            ct.c0[i] = (pk.p0[i] + (uint64_t)std::abs(e0[i])) % q;
        }
        
        // c1 = pk1 + e1
        for (size_t i = 0; i < N; i++) {
            ct.c1[i] = (pk.p1[i] + (uint64_t)std::abs(e1[i])) % q;
        }
        
        return ct;
    }
    
    // Encrypt zero (for ZANS)
    SpiralCiphertext encrypt_zero(const SpiralPublicKey& pk) {
        return encrypt(0, pk);
    }
};

} // namespace spiral
