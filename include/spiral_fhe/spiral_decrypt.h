// SPIRAL-DECRYPT v3 — Working Decryption
// m = (c0 + c1·s)[0] mod q mod t

#pragma once
#include "spiral_fhe.h"
#include "spiral_ntt.h"

namespace spiral {

class SpiralDecrypt {
private:
    SpiralParams params;
    SpiralNTT ntt;
    
public:
    SpiralDecrypt(const SpiralParams& p = SpiralParams()) 
        : params(p), ntt(p.poly_degree) {}
    
    uint64_t get_modulus() const { return ntt.modulus(); }
    
    uint64_t decrypt(const SpiralCiphertext& ct, const SpiralSecretKey& sk) {
        size_t N = params.poly_degree;
        uint64_t q = ntt.modulus();
        uint64_t t = params.plain_modulus;
        
        // Convert sk to NTT-compatible vector
        std::vector<uint64_t> s_vec(N, 0);
        for (size_t i = 0; i < N; i++) {
            int64_t val = sk.s[i];
            s_vec[i] = (val < 0) ? (q - (uint64_t)(-val)) : (uint64_t)val;
            s_vec[i] %= q;
        }
        
        // c1·s using NTT
        auto c1s = ntt.multiply(ct.c1, s_vec);
        
        // m = (c0 + c1·s)[0] mod q mod t
        uint64_t result = (ct.c0[0] + c1s[0]) % q;
        result = result % t;
        
        return result;
    }
    
    int noise_budget(const SpiralCiphertext& ct) {
        uint64_t max_val = 0;
        for (size_t i = 0; i < params.poly_degree; i++) {
            max_val = std::max(max_val, ct.c0[i]);
            max_val = std::max(max_val, ct.c1[i]);
        }
        uint64_t q = ntt.modulus();
        if (max_val == 0) return 60;
        return std::max(0, (int)(log2(q) - log2(max_val)));
    }
};

} // namespace spiral
