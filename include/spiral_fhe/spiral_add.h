// SPIRAL-ADD v3 — Homomorphic Addition with ZANS

#pragma once
#include "spiral_fhe.h"

namespace spiral {

class SpiralAdd {
private:
    SpiralParams params;
    
public:
    SpiralAdd(const SpiralParams& p = SpiralParams()) : params(p) {}
    
    // Homomorphic addition: ct_res = ct1 + ct2
    SpiralCiphertext add(const SpiralCiphertext& ct1, const SpiralCiphertext& ct2) {
        SpiralCiphertext result;
        size_t N = params.poly_degree;
        
        result.c0.resize(N, 0);
        result.c1.resize(N, 0);
        result.level = std::max(ct1.level, ct2.level);
        
        for (size_t i = 0; i < N; i++) {
            result.c0[i] = (ct1.c0[i] + ct2.c0[i]) % params.plain_modulus;
            result.c1[i] = (ct1.c1[i] + ct2.c1[i]) % params.plain_modulus;
        }
        
        return result;
    }
    
    // ZANS: Add Enc(0) to stabilize noise
    SpiralCiphertext zans(const SpiralCiphertext& ct, const SpiralCiphertext& enc_zero) {
        return add(ct, enc_zero);
    }
    
    // Multiple ZANS iterations
    SpiralCiphertext zans_iterations(const SpiralCiphertext& ct, 
                                       const SpiralCiphertext& enc_zero, 
                                       int iterations) {
        SpiralCiphertext result = ct;
        for (int i = 0; i < iterations; i++) {
            result = add(result, enc_zero);
        }
        return result;
    }
};

} // namespace spiral
