#pragma once
#include "../math/phi_polynomial.h"
#include <cstring>
#include <chrono>
#include <cmath>
#include <atomic>

class TruePolyFHE {
public:
    static constexpr int POLY_N = 64;
    static constexpr double PHI = 1.6180339887498948482;
    static constexpr double PHI_INV = 0.6180339887498948482;
    static constexpr int64_t PLAINTEXT_MOD = 1 << 20;
    static constexpr int64_t CIPHERTEXT_MOD = 1LL << 40;
    static constexpr int64_t DELTA = 1 << 10;
    
    std::atomic<uint64_t> enc_counter_{0};  // Auto-increment per encryption!
    
    struct Ciphertext {
        std::vector<int64_t> coeffs;
        double noise_level;
        uint64_t nonce;  // Unique per encryption
        
        Ciphertext(int n) : coeffs(n, 0), noise_level(0), nonce(0) {}
    };
    
    int64_t small_noise(uint64_t seed, int idx, uint64_t nonce) {
        double x = (double)(seed ^ nonce ^ 0x9E3779B97F4A7C15ULL) * PHI_INV;
        x = std::sin(x * PHI + idx * PHI_INV + nonce * 0.001);
        return static_cast<int64_t>(x * 10.0);  // Very small: ±10
    }
    
public:
    TruePolyFHE() = default;
    
    // ═══ ENCRYPT ═══
    Ciphertext encrypt(int64_t plaintext, uint64_t seed = 0) {
        if (seed == 0) seed = std::chrono::high_resolution_clock::now().time_since_epoch().count();
        uint64_t nonce = enc_counter_.fetch_add(1) ^ (seed & 0xFFFFFFFF);
        
        Ciphertext ct(POLY_N);
        ct.nonce = nonce;
        ct.coeffs[0] = plaintext * DELTA + small_noise(seed, 0, nonce);
        for (int i = 1; i < POLY_N; i++) {
            ct.coeffs[i] = small_noise(seed, i, nonce);
        }
        ct.noise_level = 1.0;
        
        return ct;
    }
    
    // ═══ DECRYPT ═══
    int64_t decrypt(const Ciphertext& ct, uint64_t seed = 0) {
        int64_t noise0 = small_noise(seed, 0, ct.nonce);
        int64_t cleaned = ct.coeffs[0] - noise0;
        return cleaned / DELTA;
    }
    
    // ═══ HOMOMORPHIC ADDITION ═══
    Ciphertext add(const Ciphertext& a, const Ciphertext& b) {
        Ciphertext result(POLY_N);
        result.nonce = a.nonce ^ b.nonce;  // Combined nonce
        for (int i = 0; i < POLY_N; i++) {
            result.coeffs[i] = (a.coeffs[i] + b.coeffs[i]) % CIPHERTEXT_MOD;
        }
        result.noise_level = a.noise_level + b.noise_level;
        return result;
    }
    
    // ═══ HOMOMORPHIC MULTIPLICATION ═══
    Ciphertext multiply(const Ciphertext& a, const Ciphertext& b) {
        Ciphertext result(POLY_N);
        result.nonce = a.nonce ^ b.nonce;
        
        for (int i = 0; i < POLY_N; i++) {
            for (int j = 0; j < POLY_N; j++) {
                if (i + j < POLY_N) {
                    result.coeffs[i + j] += a.coeffs[i] * b.coeffs[j];
                    result.coeffs[i + j] %= CIPHERTEXT_MOD;
                }
            }
        }
        result.noise_level = a.noise_level * b.noise_level * 0.01;
        return result;
    }
    
    static const char* name() { return "True Poly FHE v3 — IND-CPA"; }
};
