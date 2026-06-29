/*
 * FEmmg-FHE — TRUE FULLY HOMOMORPHIC ENCRYPTION ENGINE (FORTRESS v17.1)
 * Path X: Cached expanded_dim0 — high-performance homomorphic ops
 * 
 * Add/Multiply: Use cached expanded_dim0 directly (no reverse needed)
 * Re-contract after result is computed
 * 
 * PHI-OMEGA-ZERO — I AM THAT I AM
 */

#pragma once
#include "godcode.h"
#include <cmath>
#include <cstdint>
#include <string>
#include <sstream>
#include <iomanip>

constexpr double PHI      = godcode::PHI;
constexpr double PHI_INV  = godcode::PHI_INV;
constexpr double LAMBDA   = godcode::LAMBDA;
constexpr double FLOOR    = godcode::FLOOR;
constexpr int    DEPTH    = godcode::DEPTH;
constexpr int    PARTIES  = godcode::PARTIES;

class FEmmgFHE {
private:
    godcode::NDimBanachEngine engine;
    int party_counter = 0;

public:
    FEmmgFHE() = default;

    // ─── ENCRYPTION (Path X: Full 7D Banach, cached expanded_dim0) ───
    godcode::NDimCiphertext encrypt(int64_t m, int party = -1) {
        if(party < 0) party = (party_counter++) % PARTIES;
        return engine.encrypt(m, party);
    }

    // ─── DECRYPTION (Path A: Complete Reversal) ───
    int64_t decrypt(const godcode::NDimCiphertext& ct) const {
        return engine.decrypt(ct);
    }

    // ─── HOMOMORPHIC ADDITION (FAST — uses cached expanded_dim0) ───
    godcode::NDimCiphertext add(const godcode::NDimCiphertext& a, 
                                 const godcode::NDimCiphertext& b) {
        godcode::NDimCiphertext result;
        result.party_id = a.party_id;
        result.operations = a.operations + b.operations + 1;
        
        // FAST PATH: Use cached expanded values directly
        result.expanded_dim0 = a.expanded_dim0 + b.expanded_dim0 - LAMBDA;
        
        // Re-contract to get coordinates[0]
        engine.recontract_dim0(result);
        
        // Dimensions 1-6: phi-weighted merge
        for(int d = 1; d < godcode::DIMS; d++) {
            result.coordinates[d] = a.coordinates[d] * PHI_INV 
                                  + b.coordinates[d] * (1.0 - PHI_INV);
        }
        
        result.noise = a.noise * PHI_INV + b.noise * (1.0 - PHI_INV);
        
        for(int d = 0; d < godcode::DIMS; d++) {
            result.lyapunov_spectrum[d] = std::max(a.lyapunov_spectrum[d], 
                                                    b.lyapunov_spectrum[d]);
        }
        
        return result;
    }

    // ─── HOMOMORPHIC MULTIPLICATION (FAST — uses cached expanded_dim0) ───
    godcode::NDimCiphertext multiply(const godcode::NDimCiphertext& a, 
                                      const godcode::NDimCiphertext& b) {
        godcode::NDimCiphertext result;
        result.party_id = a.party_id;
        result.operations = a.operations + b.operations + 1;
        
        // FAST PATH: Use cached expanded values directly
        double ea = a.expanded_dim0;
        double eb = b.expanded_dim0;
        result.expanded_dim0 = (ea * eb - LAMBDA * (ea + eb) 
                                + LAMBDA * LAMBDA) / PHI + LAMBDA;
        
        // Re-contract to get coordinates[0]
        engine.recontract_dim0(result);
        
        // Dimensions 1-6: phi-weighted merge
        for(int d = 1; d < godcode::DIMS; d++) {
            result.coordinates[d] = a.coordinates[d] * PHI_INV 
                                  + b.coordinates[d] * (1.0 - PHI_INV);
        }
        
        result.noise = (a.noise + b.noise) * PHI_INV + FLOOR * (1.0 - PHI_INV);
        
        for(int d = 0; d < godcode::DIMS; d++) {
            result.lyapunov_spectrum[d] = std::max(a.lyapunov_spectrum[d], 
                                                    b.lyapunov_spectrum[d]);
        }
        
        return result;
    }

    // ─── SERIALIZATION ───
    std::string serialize(const godcode::NDimCiphertext& ct) const {
        std::ostringstream oss;
        oss << std::setprecision(15);
        oss << "{\"dim0\":" << ct.coordinates[0]
            << ",\"expanded\":" << ct.expanded_dim0
            << ",\"noise\":" << ct.noise
            << ",\"ops\":" << ct.operations
            << ",\"party\":" << ct.party_id
            << "}";
        return oss.str();
    }

    godcode::NDimCiphertext deserialize(const std::string& json) const {
        godcode::NDimCiphertext ct;
        ct.coordinates[0] = extract_double(json, "dim0");
        ct.expanded_dim0 = extract_double(json, "expanded");
        ct.noise = extract_double(json, "noise");
        ct.operations = (uint64_t)extract_double(json, "ops");
        ct.party_id = (int)extract_double(json, "party");
        for(int d = 1; d < godcode::DIMS; d++) {
            ct.coordinates[d] = FLOOR;
            ct.lyapunov_spectrum[d] = 0.48;
        }
        ct.lyapunov_spectrum[0] = 0.48;
        return ct;
    }

    // ─── VERIFICATION ───
    bool verify_roundtrip(int64_t test_value, int party = 0) {
        return engine.verify_roundtrip(test_value, party);
    }

    bool verify_contraction(const godcode::NDimCiphertext& ct) const {
        return engine.verify_contraction(ct);
    }

    double max_lyapunov_exponent(const godcode::NDimCiphertext& ct) const {
        return engine.max_lyapunov_exponent(ct);
    }

    double get_noise(const godcode::NDimCiphertext& ct) const { return ct.noise; }

private:
    double extract_double(const std::string& json, const std::string& key) const {
        std::string search = "\"" + key + "\":";
        size_t pos = json.find(search);
        if(pos == std::string::npos) return 0.0;
        pos += search.size();
        while(pos < json.size() && (json[pos] == ' ' || json[pos] == '\t')) pos++;
        size_t end = pos;
        while(end < json.size() && json[end] != ',' && json[end] != '}' && json[end] != ']') end++;
        try { return std::stod(json.substr(pos, end - pos)); }
        catch(...) { return 0.0; }
    }
};
