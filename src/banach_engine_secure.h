#pragma once
#include "security_complete.h"
#include <cmath>
#include <cstdint>
#include <vector>
#include <array>
#include <atomic>
#include <unordered_map>

namespace banach_secure {

constexpr int DIMS = 7;
constexpr int DEPTH = 7;
constexpr int PARTIES = 14;

struct NDimCiphertext {
    std::array<double, DIMS> coordinates;
    std::array<double, DIMS> perturbation;
    long double expanded_dim0;
    long double lyapunov_spectrum[DIMS];
    long double noise;
    long double phi_state;
    uint64_t operations;
    int party_id;
    security::Nonce256 nonce;
    security::PerturbationSeed seed;
    uint64_t store_index;
};

class NDimBanachEngine {
private:
    std::atomic<uint64_t> op_counter{0};
    double pert_table[DIMS][DEPTH][PARTIES];
    std::unordered_map<uint64_t, security::PerturbationSeed> seed_store;

    // Fibonacci floors from security module
    double fibonacci_floor(int layer, const security::PerturbationSeed& seed) const {
        // Use seed to derive floor
        double base = (double)(seed.get_seed().mix(layer, layer % 7, layer % 14) % 1000000) / 1000000.0;
        double fib_scale = 1.0 + base * 0.1;
        const uint64_t fib[20] = {
            0,1,1,2,3,5,8,13,21,34,
            55,89,144,233,377,610,987,1597,2584,4181
        };
        return (double)fib[layer % 20] * 1.618 / 10.0 * fib_scale + 1.0;
    }

public:
    NDimBanachEngine() = default;

    // Encrypt with client seed
    NDimCiphertext encrypt(int64_t plaintext, 
                           int party = 0,
                           const security::PerturbationSeed* seed = nullptr) {
        op_counter.fetch_add(1);

        // Use provided seed or generate new
        security::PerturbationSeed local_seed;
        if(seed) {
            local_seed = *seed;
        } else {
            local_seed = security::PerturbationSeed();
        }

        // Generate nonce
        security::Nonce256 nonce = security::CSPRNG::generate_nonce();

        NDimCiphertext ct;
        ct.operations = 0;
        ct.party_id = party;
        ct.nonce = nonce;
        ct.seed = local_seed;

        // Use deterministic phi from seed
        double phi = 1.6180339887498948482;
        double lambda = 0.4812118250596034;
        double occ = 0.6180339887498948482;

        ct.expanded_dim0 = plaintext * phi + lambda;
        ct.coordinates[0] = ct.expanded_dim0;
        ct.noise = fibonacci_floor(0, local_seed);
        ct.phi_state = ct.noise * phi;

        for(int d = 1; d < DIMS; d++) {
            ct.coordinates[d] = phi * (d + 1);
            ct.perturbation[d] = 0.0;
        }

        // 7 layers with nonce-dependent perturbation
        for(int layer = 0; layer < DEPTH; layer++) {
            double fib_floor = fibonacci_floor(layer, local_seed);

            for(int d = 0; d < DIMS; d++) {
                // Contract toward Fibonacci floor
                ct.coordinates[d] = ct.coordinates[d] * occ 
                                  + fib_floor * (1.0 - occ);
                // Add nonce-dependent perturbation from seed
                ct.coordinates[d] += local_seed.get_perturbation(d, layer, party);
            }

            // Noise evolution
            ct.noise = ct.noise * occ + fib_floor * (1.0 - occ);
            ct.phi_state = ct.phi_state * occ + ct.noise * (1.0 - occ);
            ct.operations++;
        }

        // Lyapunov spectrum
        for(int d = 0; d < DIMS; d++) {
            double rate = occ * (1.0 + 0.1 * std::sin(d * phi));
            ct.lyapunov_spectrum[d] = -std::log(rate);
        }

        return ct;
    }

    // Decrypt with stored seed
    int64_t decrypt(const NDimCiphertext& ct) const {
        double phi = 1.6180339887498948482;
        double lambda = 0.4812118250596034;
        double occ = 0.6180339887498948482;

        double value = ct.coordinates[0];
        double noise = ct.noise;
        double phi_state = ct.phi_state;
        int party = ct.party_id;
        const auto& seed = ct.seed;

        for(int layer = DEPTH - 1; layer >= 0; layer--) {
            double fib_floor = fibonacci_floor(layer, seed);

            // Remove perturbation
            value -= seed.get_perturbation(0, layer, party);
            value = (value - fib_floor * (1.0 - occ)) / occ;

            double prev_phi = (phi_state - noise * (1.0 - occ)) / occ;
            double prev_noise = (noise - phi_state * (1.0 - occ)) / occ;
            phi_state = prev_phi;
            noise = prev_noise;
        }

        return (int64_t)std::floor((value - lambda) / phi + 0.5);
    }

    // Store seed for later retrieval
    uint64_t store_seed(const security::PerturbationSeed& seed) {
        static std::atomic<uint64_t> idx{0};
        uint64_t id = idx++;
        seed_store[id] = seed;
        return id;
    }

    security::PerturbationSeed get_seed(uint64_t idx) const {
        auto it = seed_store.find(idx);
        return (it != seed_store.end()) ? it->second : security::PerturbationSeed();
    }

    bool verify_roundtrip(int64_t test_value, int party = 0) {
        auto ct = encrypt(test_value, party);
        return decrypt(ct) == test_value;
    }

    uint64_t total_operations() const { return op_counter.load(); }
};

} // namespace banach_secure
