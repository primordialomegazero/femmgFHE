// SPIRAL-MUL — Homomorphic Multiplication for Spiral-FHE
// ct_mul = (c0_1*c0_2, c0_1*c1_2 + c1_1*c0_2, c1_1*c1_2)
// Then relinearize: reduce 3 components back to 2

#pragma once
#include "spiral_fhe.h"
#include "spiral_ntt.h"

namespace spiral {

class SpiralMul {
private:
    SpiralParams params;
    SpiralNTT ntt;
    
public:
    SpiralMul(const SpiralParams& p = SpiralParams()) 
        : params(p), ntt(p.poly_degree) {}
    
    // Homomorphic multiplication (without relinearization)
    // Result has 3 components: (c0, c1, c2)
    std::vector<std::vector<uint64_t>> multiply_raw(
        const SpiralCiphertext& ct1, 
        const SpiralCiphertext& ct2) {
        
        size_t N = params.poly_degree;
        uint64_t q = ntt.modulus();
        
        // c0 = ct1.c0 * ct2.c0
        auto c0 = ntt.multiply(ct1.c0, ct2.c0);
        
        // c1 = ct1.c0 * ct2.c1 + ct1.c1 * ct2.c0
        auto c0_times_c1_2 = ntt.multiply(ct1.c0, ct2.c1);
        auto c1_times_c0_2 = ntt.multiply(ct1.c1, ct2.c0);
        std::vector<uint64_t> c1(N, 0);
        for (size_t i = 0; i < N; i++) {
            c1[i] = (c0_times_c1_2[i] + c1_times_c0_2[i]) % q;
        }
        
        // c2 = ct1.c1 * ct2.c1
        auto c2 = ntt.multiply(ct1.c1, ct2.c1);
        
        return {c0, c1, c2};
    }
    
    // Decrypt multiplication result
    // m1·m2 = c0 + c1·s + c2·s²
    uint64_t decrypt_mul(
        const std::vector<std::vector<uint64_t>>& ct_mul,
        const SpiralSecretKey& sk) {
        
        size_t N = params.poly_degree;
        uint64_t q = ntt.modulus();
        uint64_t t = params.plain_modulus;
        
        // Convert sk to vector
        std::vector<uint64_t> s_vec(N, 0);
        for (size_t i = 0; i < N; i++) {
            int64_t val = sk.s[i];
            s_vec[i] = (val < 0) ? (q - (uint64_t)(-val)) : (uint64_t)val;
        }
        
        // s² = s * s
        auto s_sq = ntt.multiply(s_vec, s_vec);
        
        // c1·s
        auto c1s = ntt.multiply(ct_mul[1], s_vec);
        
        // c2·s²
        auto c2s2 = ntt.multiply(ct_mul[2], s_sq);
        
        // m1·m2 = (c0 + c1·s + c2·s²)[0] mod q mod t
        uint64_t result = (ct_mul[0][0] + c1s[0] + c2s2[0]) % q;
        result = result % t;
        
        return result;
    }
};

} // namespace spiral
