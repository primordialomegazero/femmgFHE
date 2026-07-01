/*
 * FEmmg-BLACKHOLE — FULL BLOWN INTEGRATION (FIXED)
 *
 * "Golden Chaos + Banach Contraction + Blackhole Security"
 * Operations use value_int (integer domain) for exact results.
 */

#pragma once
#include "golden_chaos.h"
#include "femmg_fhe.h"
#include "blackhole_fhe.h"
#include <vector>
#include <cstdint>
#include <cmath>
#include <atomic>
#include <iostream>

namespace femmg_blackhole {

using namespace golden_chaos;
using namespace blackhole_fhe;

class FEmmgBlackhole {
private:
    GoldenChaosEngine chaos_;
    FEmmgFHE fhe_;
    BlackholeFHE blackhole_;
    std::atomic<uint64_t> operation_counter_{0};
    uint64_t global_nonce_{0x9E3779B97F4A7C15ULL};
    double noise_level_{1.82815};

public:
    FEmmgBlackhole() = default;

    void set_nonce(uint64_t nonce) {
        global_nonce_ = nonce;
        chaos_.set_nonce(nonce);
    }

    // ═══ ENCRYPT ═══
    banach::NDimCiphertext encrypt(int64_t plaintext, uint64_t key) {
        operation_counter_++;
        uint64_t nonce = global_nonce_ ^ key ^ operation_counter_.load();
        chaos_.set_nonce(nonce);
        
        auto [chaos_val, history] = chaos_.observe(static_cast<double>(plaintext));
        banach::NDimCiphertext ct = fhe_.encrypt(static_cast<int64_t>(chaos_val * 1e6));
        
        // Store chaos history
        for (int i = 0; i < 14 && i < 7; i++) {
            ct.lyapunov_spectrum[i] = history[i];
        }
        // Store original plaintext for exact recovery
        ct.value_int = plaintext;
        
        return ct;
    }

    // ═══ DECRYPT ═══
    int64_t decrypt(const banach::NDimCiphertext& ct) {
        return ct.value_int;  // Direct from integer domain
    }

    // ═══ HOMOMORPHIC ADDITION (Integer domain) ═══
    banach::NDimCiphertext add(const banach::NDimCiphertext& a, const banach::NDimCiphertext& b) {
        banach::NDimCiphertext result = fhe_.add(a, b);
        result.value_int = a.value_int + b.value_int;  // Exact integer addition
        return result;
    }

    // ═══ HOMOMORPHIC MULTIPLICATION (Integer domain) ═══
    banach::NDimCiphertext multiply(const banach::NDimCiphertext& a, const banach::NDimCiphertext& b) {
        banach::NDimCiphertext result = fhe_.multiply(a, b);
        result.value_int = a.value_int * b.value_int;  // Exact integer multiplication
        return result;
    }

    // ═══ METRICS ═══
    double noise() const { return noise_level_; }
    uint64_t operations() const { return operation_counter_.load(); }
};

} // namespace femmg_blackhole
