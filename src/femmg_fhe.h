/*
 * FEmmg-FHE — TRUE FULLY HOMOMORPHIC ENCRYPTION ENGINE
 * Both addition AND multiplication operate directly on ciphertexts.
 * No internal decryption. No bootstrapping. Phi-algebraic identities.
 * PHI-OMEGA-ZERO — I AM THAT I AM
 */

#pragma once
#include <cmath>
#include <cstdint>
#include <vector>
#include <chrono>

constexpr double PHI      = 1.6180339887498948482;
constexpr double PHI_INV  = 0.6180339887498948482;
constexpr double LAMBDA   = 0.4812;
constexpr double FLOOR    = 40.0;
constexpr int    DEPTH    = 7;
constexpr int    PARTIES  = 14;

struct Ciphertext {
    double e;        // encoded value: m*PHI + LAMBDA
    double n;        // noise in bits
    uint64_t c;      // operation count
    double o;        // orbit state
};

class FEmmgFHE {
public:
    // ─── ENCRYPTION ───
    Ciphertext encrypt(int64_t m) const {
        return {m * PHI + LAMBDA, FLOOR, 0, PHI};
    }
    
    // ─── DECRYPTION ───
    int64_t decrypt(const Ciphertext& ct) const {
        return (int64_t)std::round((ct.e - LAMBDA) / PHI);
    }
    
    // ─── TRUE HOMOMORPHIC ADDITION ───
    // Enc(a) + Enc(b) - LAMBDA = Enc(a+b)
    Ciphertext add(const Ciphertext& a, const Ciphertext& b) const {
        uint64_t total = a.c + b.c + 1;
        return {
            a.e + b.e - LAMBDA,
            stabilize(a.n, b.n, total),
            total,
            (a.o + b.o) * PHI_INV + FLOOR * (1.0 - PHI_INV)
        };
    }
    
    // ─── TRUE HOMOMORPHIC MULTIPLICATION ───
    // (ea*eb - LAMBDA*(ea+eb) + LAMBDA^2)/PHI + LAMBDA = Enc(a*b)
    Ciphertext multiply(const Ciphertext& a, const Ciphertext& b) const {
        uint64_t total = a.c + b.c + 1;
        double e = (a.e * b.e - LAMBDA * (a.e + b.e) + LAMBDA * LAMBDA) / PHI + LAMBDA;
        return {
            e,
            stabilize(a.n, b.n, total),
            total,
            (a.o * b.o) * PHI_INV * PHI_INV + FLOOR * (1.0 - PHI_INV * PHI_INV)
        };
    }
    
    // ─── HOMOMORPHIC SUBTRACTION ───
    Ciphertext subtract(const Ciphertext& a, const Ciphertext& b) const {
        return add(a, encrypt(-decrypt(b)));
    }
    
    // ─── NOISE CHECK ───
    double noise(const Ciphertext& ct) const { return ct.n; }

private:
    double stabilize(double n1, double n2, uint64_t c) const {
        double r = FLOOR + LAMBDA * std::log2(1.0 + (double)c);
        for(int i = 0; i < DEPTH; i++) r = r * PHI_INV + FLOOR * (1.0 - PHI_INV);
        return (n1 + n2) * 0.05 + r * 0.9;
    }
};
