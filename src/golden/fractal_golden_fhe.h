#pragma once
#include <cmath>
#include <array>
#include <cstdint>

namespace FractalGoldenFHE {

constexpr double PHI = 1.6180339887498948482;
constexpr double PSI = -0.6180339887498948482;
constexpr int MAX_DIM = 16;
constexpr int MAX_DEPTH = 4;

inline double swing(double v) { return -1.0 / v; }

inline uint64_t golden_prng(uint64_t seed) {
    uint64_t state = seed;
    for (int i = 0; i < 12; i++) {
        state ^= (state << 13);
        state ^= (state >> 7);
        state ^= (state << 17);
        state = (state * 0x9e3779b97f4a7c15ULL) ^ 0xbf58476d1ce4e5b9ULL;
    }
    return state;
}

class Cipher {
public:
    std::array<std::array<double, MAX_DIM>, MAX_DEPTH> fractal;
    int swing_count;
    uint64_t nonce;

    Cipher() : swing_count(0), nonce(0) {
        for (auto& layer : fractal) layer.fill(0.0);
    }
};

inline Cipher encrypt(bool bit, int secret_swings, uint64_t secret_seed) {
    Cipher ct;
    double base = bit ? PHI : PSI;
    uint64_t nonce = golden_prng(secret_seed ^ (bit ? 0x123456789abcdef0ULL : 0xfedcba9876543210ULL));

    for (int depth = 0; depth < MAX_DEPTH; depth++) {
        for (int dim = 0; dim < MAX_DIM; dim++) {
            double v = base;
            double perturb = 1.0 + (nonce & 0xFFFFFF) * 1e-15;
            v *= perturb;
            for (int i = 0; i < secret_swings; i++) v = swing(v);
            ct.fractal[depth][dim] = v;
            nonce = golden_prng(nonce);
        }
    }

    ct.swing_count = secret_swings;
    ct.nonce = nonce;
    return ct;
}

inline bool decrypt(const Cipher& ct) {
    int positives = 0;
    int total = 0;
    for (int depth = 0; depth < MAX_DEPTH; depth++) {
        for (int dim = 0; dim < MAX_DIM; dim++) {
            double recovered = ct.fractal[depth][dim];
            for (int i = 0; i < ct.swing_count; i++) recovered = swing(recovered);
            if (recovered > 0) positives++;
            total++;
        }
    }
    return positives > total / 2;
}

inline Cipher nand(const Cipher& a, const Cipher& b) {
    Cipher r;
    double one = PHI + PSI;
    for (int depth = 0; depth < MAX_DEPTH; depth++) {
        for (int dim = 0; dim < MAX_DIM; dim++) {
            r.fractal[depth][dim] = one - a.fractal[depth][dim] * b.fractal[depth][dim];
        }
    }
    r.swing_count = a.swing_count;
    r.nonce = a.nonce ^ b.nonce;
    return r;
}

inline Cipher bootstrap(const Cipher& ct) {
    Cipher r = ct;
    for (int depth = 0; depth < MAX_DEPTH; depth++) {
        for (int dim = 0; dim < MAX_DIM; dim++) {
            double v = ct.fractal[depth][dim];
            if (v > 0.5) v = PHI;
            else v = PSI;
            r.fractal[depth][dim] = v;
        }
    }
    return r;
}

inline bool verify_fractal(const Cipher& ct) {
    for (int depth = 0; depth < MAX_DEPTH; depth++) {
        for (int dim = 0; dim < MAX_DIM; dim++) {
            double v = ct.fractal[depth][dim];
            if (std::abs(v - PHI) > 0.1 && std::abs(v - PSI) > 0.1) return false;
        }
    }
    return true;
}

} // namespace FractalGoldenFHE
