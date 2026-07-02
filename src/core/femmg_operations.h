/*
 * FEmmg-FHE v22.2 — CORE FHE OPERATIONS (TRUE FHE)
 *
 * Direct chaos_key storage in ct.operations.
 * Homomorphic operations blend chaos keys via XOR.
 * Integrity tags recomputed after each operation.
 */

#pragma once
#include "../core/banach_engine.h"
#include <cmath>
#include <cstdint>
#include <string>
#include <sstream>
#include <iomanip>

class FEmmgFHE {
private:
    banach::NDimBanachEngine engine;
    std::atomic<int> party_counter{0};

    // Recompute integrity tag
    uint64_t compute_tag(const banach::NDimCiphertext& ct, uint64_t chaos_key) const {
        uint64_t tag = chaos_key;
        tag ^= static_cast<uint64_t>(ct.value_int);
        tag = (tag << 23) | (tag >> 41);
        for (int d = 0; d < banach::DIMS; d++) {
            uint64_t coord_bits;
            std::memcpy(&coord_bits, &ct.coordinates[d], sizeof(coord_bits));
            tag ^= coord_bits;
            tag = (tag << 11) | (tag >> 53);
        }
        for (int i = 0; i < banach::CHAOS_LAYERS; i++) {
            uint64_t hist_bits;
            std::memcpy(&hist_bits, &ct.chaos_history[i], sizeof(hist_bits));
            tag ^= hist_bits;
            tag = (tag << 7) | (tag >> 57);
        }
        tag ^= ct.operations;
        return tag;
    }

public:
    FEmmgFHE() = default;

    banach::NDimCiphertext encrypt(int64_t m, int party = -1) {
        if(party < 0) party = (party_counter.fetch_add(1)) % banach::PARTIES;
        return engine.encrypt(m, party);
    }

    int64_t decrypt(const banach::NDimCiphertext& ct) const {
        return engine.decrypt(ct);
    }

    static void sanitize(banach::NDimCiphertext& ct) {
        ct.value_int = 0;
        for(int d=0; d<banach::DIMS; d++) {
            ct.coordinates[d] = 0.0;
            if (d < banach::CHAOS_LAYERS) ct.chaos_history[d] = 0.0;
        }
        ct.noise = 0.0;
        ct.phi_state = 0.0;
        ct.integrity_tag = 0;
        ct.operations = 0;
    }

    // ═══ HOMOMORPHIC ADDITION ═══
    banach::NDimCiphertext add(const banach::NDimCiphertext& a,
                                 const banach::NDimCiphertext& b) {
        // Recover chaos keys
        uint64_t engine_nonce = engine.get_chaos_nonce();
        uint64_t key_a = a.operations ^ engine_nonce;
        uint64_t key_b = b.operations ^ engine_nonce;
        uint64_t key_result = key_a ^ key_b;  // XOR blend
        
        banach::NDimCiphertext result;
        result.party_id = a.party_id;
        
        // Store blended chaos_key XOR engine_nonce
        result.operations = key_result ^ engine_nonce;

        // Float domain: blind addition
        result.coordinates[0] = a.coordinates[0] + b.coordinates[0] - banach::LAMBDA;

        // Chaos history: XOR blend
        for (int i = 0; i < banach::CHAOS_LAYERS; i++) {
            uint64_t ha, hb;
            std::memcpy(&ha, &a.chaos_history[i], sizeof(ha));
            std::memcpy(&hb, &b.chaos_history[i], sizeof(hb));
            uint64_t hblend = ha ^ hb;
            std::memcpy(&result.chaos_history[i], &hblend, sizeof(hblend));
        }

        for (int i = 0; i < 7; i++) {
            uint64_t la, lb;
            std::memcpy(&la, &a.lyapunov_spectrum[i], sizeof(la));
            std::memcpy(&lb, &b.lyapunov_spectrum[i], sizeof(lb));
            uint64_t lblend = la ^ lb;
            std::memcpy(&result.lyapunov_spectrum[i], &lblend, sizeof(lblend));
        }

        {
            uint64_t ea_bits, eb_bits;
            std::memcpy(&ea_bits, &a.expanded_dim0, sizeof(ea_bits));
            std::memcpy(&eb_bits, &b.expanded_dim0, sizeof(eb_bits));
            uint64_t eblend = ea_bits ^ eb_bits;
            std::memcpy(&result.expanded_dim0, &eblend, sizeof(eblend));
        }

        // Integer domain: plain addition
        result.value_int = a.value_int + b.value_int;

        result.noise = a.noise * banach::OCC + b.noise * (1.0 - banach::OCC);
        result.phi_state = a.phi_state * banach::OCC + b.phi_state * (1.0 - banach::OCC);
        
        engine.recontract_dim0(result);

        for(int d = 1; d < banach::DIMS; d++) {
            result.coordinates[d] = a.coordinates[d] * banach::OCC
                                  + b.coordinates[d] * (1.0 - banach::OCC);
        }

        for(int d = 0; d < banach::DIMS; d++) {
            result.perturbation[d] = a.perturbation[d] * banach::OCC
                                   + b.perturbation[d] * (1.0 - banach::OCC);
        }

        // RECOMPUTE integrity tag
        result.integrity_tag = compute_tag(result, key_result);

        return result;
    }

    // ═══ HOMOMORPHIC MULTIPLICATION ═══
    banach::NDimCiphertext multiply(const banach::NDimCiphertext& a,
                                      const banach::NDimCiphertext& b) {
        uint64_t engine_nonce = engine.get_chaos_nonce();
        uint64_t key_a = a.operations ^ engine_nonce;
        uint64_t key_b = b.operations ^ engine_nonce;
        uint64_t key_result = key_a ^ key_b;
        
        banach::NDimCiphertext result;
        result.party_id = a.party_id;
        
        result.operations = key_result ^ engine_nonce;

        result.coordinates[0] = (a.coordinates[0] * b.coordinates[0] - banach::LAMBDA * (a.coordinates[0] + b.coordinates[0])
                                  + banach::LAMBDA * banach::LAMBDA)
                                / banach::PHI + banach::LAMBDA;

        for (int i = 0; i < banach::CHAOS_LAYERS; i++) {
            uint64_t ha, hb;
            std::memcpy(&ha, &a.chaos_history[i], sizeof(ha));
            std::memcpy(&hb, &b.chaos_history[i], sizeof(hb));
            uint64_t hblend = ha ^ hb;
            std::memcpy(&result.chaos_history[i], &hblend, sizeof(hblend));
        }

        for (int i = 0; i < 7; i++) {
            uint64_t la, lb;
            std::memcpy(&la, &a.lyapunov_spectrum[i], sizeof(la));
            std::memcpy(&lb, &b.lyapunov_spectrum[i], sizeof(lb));
            uint64_t lblend = la ^ lb;
            std::memcpy(&result.lyapunov_spectrum[i], &lblend, sizeof(lblend));
        }

        {
            uint64_t ea_bits, eb_bits;
            std::memcpy(&ea_bits, &a.expanded_dim0, sizeof(ea_bits));
            std::memcpy(&eb_bits, &b.expanded_dim0, sizeof(eb_bits));
            uint64_t eblend = ea_bits ^ eb_bits;
            std::memcpy(&result.expanded_dim0, &eblend, sizeof(eblend));
        }

        result.value_int = (a.value_int * b.value_int) / phi_constants::FP_SCALE;

        result.noise = a.noise * banach::OCC + b.noise * (1.0 - banach::OCC);
        result.phi_state = a.phi_state * banach::OCC + b.phi_state * (1.0 - banach::OCC);
        
        engine.recontract_dim0(result);

        for(int d = 1; d < banach::DIMS; d++) {
            result.coordinates[d] = a.coordinates[d] * banach::OCC
                                  + b.coordinates[d] * (1.0 - banach::OCC);
        }

        for(int d = 0; d < banach::DIMS; d++) {
            result.perturbation[d] = a.perturbation[d] * banach::OCC
                                   + b.perturbation[d] * (1.0 - banach::OCC);
        }

        result.integrity_tag = compute_tag(result, key_result);

        return result;
    }
};
