// SPIRAL-KEYGEN v2 — NTT-compatible Key Generation
#pragma once
#include "spiral_fhe.h"
#include "spiral_ntt.h"
#include <random>

namespace spiral {

class SpiralKeyGen {
private:
    SpiralParams params;
    SpiralNTT ntt;
    std::mt19937_64 rng;
    
public:
    SpiralKeyGen(const SpiralParams& p = SpiralParams()) 
        : params(p), ntt(p.poly_degree) {
        rng.seed(1618033988);
    }
    
    uint64_t get_modulus() const { return ntt.modulus(); }
    
    SpiralSecretKey generate_secret_key() {
        SpiralSecretKey sk;
        size_t N = params.poly_degree;
        sk.s.resize(N, 0);
        
        size_t weight = N / 3;
        std::uniform_int_distribution<size_t> pos_dist(0, N - 1);
        std::uniform_int_distribution<int> sign_dist(0, 1);
        
        for (size_t i = 0; i < weight; i++) {
            size_t pos = pos_dist(rng);
            sk.s[pos] = sign_dist(rng) ? 1 : -1;
        }
        
        return sk;
    }
    
    SpiralPublicKey generate_public_key(const SpiralSecretKey& sk) {
        SpiralPublicKey pk;
        size_t N = params.poly_degree;
        uint64_t q = ntt.modulus();
        
        pk.p0.resize(N, 0);
        pk.p1.resize(N, 0);
        
        // Generate random polynomial a
        std::uniform_int_distribution<uint64_t> a_dist(0, q - 1);
        for (size_t i = 0; i < N; i++) {
            pk.p1[i] = a_dist(rng);
        }
        
        // Convert sk to vector
        std::vector<uint64_t> s_vec(N, 0);
        for (size_t i = 0; i < N; i++) {
            int64_t val = sk.s[i];
            s_vec[i] = (val < 0) ? (q - (uint64_t)(-val)) : (uint64_t)val;
            s_vec[i] %= q;
        }
        
        // a·s using NTT
        auto a_times_s = ntt.multiply(pk.p1, s_vec);
        
        // Noise
        SpiralNoiseGenerator noise_gen;
        auto e = noise_gen.generate_vector(N);
        
        // p0 = -(a·s) + e (mod q)
        for (size_t i = 0; i < N; i++) {
            int64_t e_int = (int64_t)e[i];
            uint64_t e_mod = (e_int < 0) ? (q - (uint64_t)(-e_int) % q) : ((uint64_t)e_int % q);
            pk.p0[i] = (e_mod + (q - a_times_s[i])) % q;
        }
        
        return pk;
    }
    
    SpiralKeyPair generate() {
        SpiralKeyPair kp;
        kp.sk = generate_secret_key();
        kp.pk = generate_public_key(kp.sk);
        return kp;
    }
};

} // namespace spiral
