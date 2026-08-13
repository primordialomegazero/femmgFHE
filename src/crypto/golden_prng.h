#pragma once
#include "../core/constants.h"
#include "../utils/safe_math.h"
#include <cstdint>
#include <cmath>

// ================================================================
// GOLDEN PRNG — Deterministic random from φ·ψ = -1
// ================================================================
class GoldenPRNG {
private:
    double state;
public:
    GoldenPRNG(double seed = 42.0) : state(SafeMath::fmod_safe(std::abs(seed))) {}

    uint64_t next_u64() {
        double a = state;
        double b = SafeMath::fmod_safe(state + PHI);
        for (int i = 0; i < 10; i++) {
            double t = SafeMath::fmod_safe((a + b) * PHI);
            a = b;
            b = t;
        }
        state = b;
        return static_cast<uint64_t>(state * 1.8446744073709552e19);
    }

    void fill(void* buffer, size_t size) {
        uint8_t* bytes = static_cast<uint8_t*>(buffer);
        size_t filled = 0;
        while (filled < size) {
            uint64_t val = next_u64();
            size_t n = (size - filled < 8) ? (size - filled) : 8;
            for (size_t i = 0; i < n; i++) {
                bytes[filled + i] = static_cast<uint8_t>((val >> (8 * i)) & 0xFF);
            }
            filled += n;
        }
    }
};
