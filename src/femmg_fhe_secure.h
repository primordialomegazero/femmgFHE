#pragma once
#include "banach_engine_secure.h"
#include "security_complete.h"
#include <unordered_map>
#include <atomic>

class FEmmgFHESecure {
private:
    banach_secure::NDimBanachEngine engine;
    std::unordered_map<uint64_t, banach_secure::NDimCiphertext> store;
    std::unordered_map<uint64_t, security::PerturbationSeed> seed_store;
    std::atomic<uint64_t> idx_counter{0};

public:
    FEmmgFHESecure() = default;

    // Client-side encryption with seed
    banach_secure::NDimCiphertext encrypt(int64_t plaintext, 
                                          int party = 0,
                                          const security::PerturbationSeed* seed = nullptr) {
        return engine.encrypt(plaintext, party, seed);
    }

    // Decryption
    int64_t decrypt(const banach_secure::NDimCiphertext& ct) const {
        return engine.decrypt(ct);
    }

    // Store with seed tracking
    uint64_t store(const banach_secure::NDimCiphertext& ct) {
        uint64_t idx = idx_counter++;
        store[idx] = ct;
        seed_store[idx] = ct.seed;
        return idx;
    }

    // Retrieve ciphertext by index
    banach_secure::NDimCiphertext get(uint64_t idx) const {
        auto it = store.find(idx);
        return (it != store.end()) ? it->second : banach_secure::NDimCiphertext();
    }

    // Get stored seed
    security::PerturbationSeed get_seed(uint64_t idx) const {
        auto it = seed_store.find(idx);
        return (it != seed_store.end()) ? it->second : security::PerturbationSeed();
    }

    // Blind addition
    banach_secure::NDimCiphertext add(const banach_secure::NDimCiphertext& a,
                                      const banach_secure::NDimCiphertext& b) {
        banach_secure::NDimCiphertext result;
        result.party_id = a.party_id;
        result.operations = a.operations + b.operations + 1;
        result.seed = a.seed; // Use first operand's seed
        result.nonce = security::CSPRNG::generate_nonce();

        // Blind add using expanded values
        long double ea = a.expanded_dim0;
        long double eb = b.expanded_dim0;
        result.expanded_dim0 = ea + eb - 0.4812118250596034L;

        // Recontract with seed
        result.noise = a.noise * 0.6180339887498948482L 
                     + b.noise * (1.0L - 0.6180339887498948482L);
        result.phi_state = a.phi_state * 0.6180339887498948482L 
                         + b.phi_state * (1.0L - 0.6180339887498948482L);

        // Recontract
        double phi = 1.6180339887498948482;
        double occ = 0.6180339887498948482;
        double value = result.expanded_dim0;
        double noise = result.noise;
        double phi_state = result.phi_state;

        for(int layer = 0; layer < banach_secure::DEPTH; layer++) {
            double fib_floor = engine.fibonacci_floor(layer, result.seed);
            value = value * occ + fib_floor * (1.0 - occ);
            value += result.seed.get_perturbation(0, layer, result.party_id);
            noise = noise * occ + fib_floor * (1.0 - occ);
            phi_state = phi_state * occ + noise * (1.0 - occ);
        }

        result.coordinates[0] = value;
        result.noise = noise;
        result.phi_state = phi_state;

        for(int d = 1; d < banach_secure::DIMS; d++) {
            result.coordinates[d] = a.coordinates[d] * occ 
                                  + b.coordinates[d] * (1.0 - occ);
        }

        return result;
    }

    // Blind multiplication
    banach_secure::NDimCiphertext multiply(const banach_secure::NDimCiphertext& a,
                                           const banach_secure::NDimCiphertext& b) {
        banach_secure::NDimCiphertext result;
        result.party_id = a.party_id;
        result.operations = a.operations + b.operations + 1;
        result.seed = a.seed;
        result.nonce = security::CSPRNG::generate_nonce();

        long double ea = a.expanded_dim0;
        long double eb = b.expanded_dim0;
        double lambda = 0.4812118250596034;
        double phi = 1.6180339887498948482;

        // Blind multiply formula
        result.expanded_dim0 = (ea * eb - lambda * (ea + eb)
                                + lambda * lambda) / phi + lambda;

        result.noise = (a.noise + b.noise) * 0.6180339887498948482L;
        result.phi_state = (a.phi_state + b.phi_state) * 0.6180339887498948482L;

        // Recontract
        double occ = 0.6180339887498948482;
        double value = result.expanded_dim0;
        double noise = result.noise;
        double phi_state = result.phi_state;

        for(int layer = 0; layer < banach_secure::DEPTH; layer++) {
            double fib_floor = engine.fibonacci_floor(layer, result.seed);
            value = value * occ + fib_floor * (1.0 - occ);
            value += result.seed.get_perturbation(0, layer, result.party_id);
            noise = noise * occ + fib_floor * (1.0 - occ);
            phi_state = phi_state * occ + noise * (1.0 - occ);
        }

        result.coordinates[0] = value;
        result.noise = noise;
        result.phi_state = phi_state;

        for(int d = 1; d < banach_secure::DIMS; d++) {
            result.coordinates[d] = a.coordinates[d] * occ 
                                  + b.coordinates[d] * (1.0 - occ);
        }

        return result;
    }

    bool verify_roundtrip(int64_t tv, int p = 0) {
        return engine.verify_roundtrip(tv, p);
    }

    uint64_t total_operations() const { return engine.total_operations(); }
};
