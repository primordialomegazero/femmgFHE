// SPIRAL-LWE v2 — Small noise, proper encoding
#pragma once
#include "spiral_ring.h"
#include <utility>
#include <random>

namespace spiral {

class SpiralLWE {
private:
    SpiralRing ring;
    uint64_t q;
    std::mt19937_64 rng;
    
    // Small noise element (coefficients bounded by noise_bound)
    SpiralRing::Element small_noise(uint64_t bound = 2) {
        std::uniform_int_distribution<uint64_t> dist(0, bound);
        return SpiralRing::Element(dist(rng), dist(rng));
    }
    
    // Small random element (for u in encryption)
    SpiralRing::Element small_random(uint64_t bound = 2) {
        return small_noise(bound);
    }
    
public:
    using SecretKey = SpiralRing::Element;
    using PublicKey = std::pair<SpiralRing::Element, SpiralRing::Element>;
    using Ciphertext = std::pair<SpiralRing::Element, SpiralRing::Element>;
    
    SpiralLWE(uint64_t modulus = 1618033988) : ring(modulus), q(modulus) {
        rng.seed(1618033988);
    }
    
    uint64_t modulus() const { return q; }
    
    // === KEY GENERATION ===
    std::pair<SecretKey, PublicKey> keygen() {
        SecretKey s = small_random(2);  // Small secret key
        SpiralRing::Element a = ring.random();  // Uniform random a
        SpiralRing::Element e = small_noise(2);  // Small noise
        
        SpiralRing::Element a_s = ring.mul(a, s);
        SpiralRing::Element p0 = ring.add(ring.neg(a_s), e);
        SpiralRing::Element p1 = a;
        
        return {s, {p0, p1}};
    }
    
    // === ENCRYPTION ===
    Ciphertext encrypt(uint64_t message, const PublicKey& pk) {
        SpiralRing::Element m = ring.encode(message);
        SpiralRing::Element u = small_random(2);
        SpiralRing::Element e0 = small_noise(2);
        SpiralRing::Element e1 = small_noise(2);
        
        SpiralRing::Element p0_u = ring.mul(pk.first, u);
        SpiralRing::Element c0 = ring.add(m, ring.add(p0_u, e0));
        
        SpiralRing::Element p1_u = ring.mul(pk.second, u);
        SpiralRing::Element c1 = ring.add(p1_u, e1);
        
        return {c0, c1};
    }
    
    // === DECRYPTION ===
    uint64_t decrypt(const Ciphertext& ct, const SecretKey& sk) {
        SpiralRing::Element c1_s = ring.mul(ct.second, sk);
        SpiralRing::Element result = ring.add(ct.first, c1_s);
        // The message is in the 'a' component
        return result.a % q;
    }
    
    // === ADDITION ===
    Ciphertext add(const Ciphertext& ct1, const Ciphertext& ct2) {
        return {ring.add(ct1.first, ct2.first), ring.add(ct1.second, ct2.second)};
    }
    
    // === MULTIPLICATION ===
    std::tuple<SpiralRing::Element, SpiralRing::Element, SpiralRing::Element> 
    multiply_raw(const Ciphertext& ct1, const Ciphertext& ct2) {
        auto c00 = ring.mul(ct1.first, ct2.first);
        auto c01 = ring.mul(ct1.first, ct2.second);
        auto c10 = ring.mul(ct1.second, ct2.first);
        auto c11 = ring.mul(ct1.second, ct2.second);
        auto c1 = ring.add(c01, c10);
        return {c00, c1, c11};
    }
    
    uint64_t decrypt_mul(
        const std::tuple<SpiralRing::Element, SpiralRing::Element, SpiralRing::Element>& ct_mul,
        const SecretKey& sk) {
        auto [c0, c1, c2] = ct_mul;
        SpiralRing::Element s_sq = ring.mul(sk, sk);
        auto c1_s = ring.mul(c1, sk);
        auto c2_s2 = ring.mul(c2, s_sq);
        auto result = ring.add(c0, ring.add(c1_s, c2_s2));
        return result.a % q;
    }
    
    // === ZANS ===
    Ciphertext zans(const Ciphertext& ct, const PublicKey& pk, int iterations = 1) {
        Ciphertext result = ct;
        Ciphertext enc_zero = encrypt(0, pk);
        for (int i = 0; i < iterations; i++) {
            result = add(result, enc_zero);
        }
        return result;
    }
};

} // namespace spiral
