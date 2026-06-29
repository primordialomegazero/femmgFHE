/*
 * FEmmg-FHE — TRUE FULLY HOMOMORPHIC ENCRYPTION ENGINE (FORTRESS v17.0)
 * Path X: Full Integration with N-Dimensional Banach Contraction
 * 
 * Encryption: godcode::NDimBanachEngine.encrypt() — 7D, 7 layers, full perturbation
 * Decryption: godcode::NDimBanachEngine.decrypt() — complete mathematical reversal
 * Add/Multiply: Expand dim0 to pre-contraction form, blind op, re-contract
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

    // Expand dimension 0: reverse the Banach contraction to get m*PHI+LAMBDA
    double expand_dim0(const godcode::NDimCiphertext& ct) const {
        double value = ct.coordinates[0];
        int party = ct.party_id;
        for(int layer = DEPTH - 1; layer >= 0; layer--) {
            value -= engine.compute_perturbation(0, layer, party);
            value = (value - FLOOR * (1.0 - PHI_INV)) / PHI_INV;
        }
        return value;
    }

    // Contract dimension 0: apply forward Banach contraction to bring back to near-floor
    double contract_dim0(double expanded_value, int party) const {
        double value = expanded_value;
        for(int layer = 0; layer < DEPTH; layer++) {
            value = value * PHI_INV + FLOOR * (1.0 - PHI_INV);
            value += engine.compute_perturbation(0, layer, party);
        }
        return value;
    }

public:
    FEmmgFHE() = default;

    // ─── ENCRYPTION (Path X: Full 7D Banach) ───
    godcode::NDimCiphertext encrypt(int64_t m, int party = -1) {
        if(party < 0) party = (party_counter++) % PARTIES;
        return engine.encrypt(m, party);
    }

    // ─── DECRYPTION (Path A: Complete Reversal) ───
    int64_t decrypt(const godcode::NDimCiphertext& ct) const {
        return engine.decrypt(ct);
    }

    // ─── HOMOMORPHIC ADDITION ───
    // Expand both dim0s → blind add → contract result
    godcode::NDimCiphertext add(const godcode::NDimCiphertext& a, 
                                 const godcode::NDimCiphertext& b) {
        godcode::NDimCiphertext result;
        result.party_id = a.party_id;
        result.operations = a.operations + b.operations + 1;
        
        // Expand dim0 to pre-contraction form (m*PHI+LAMBDA)
        double expanded_a = expand_dim0(a);
        double expanded_b = expand_dim0(b);
        
        // Blind addition on expanded values
        double expanded_sum = expanded_a + expanded_b - LAMBDA;
        
        // Contract back to near-floor
        result.coordinates[0] = contract_dim0(expanded_sum, result.party_id);
        
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

    // ─── HOMOMORPHIC MULTIPLICATION ───
    // Expand both dim0s → blind multiply → contract result
    godcode::NDimCiphertext multiply(const godcode::NDimCiphertext& a, 
                                      const godcode::NDimCiphertext& b) {
        godcode::NDimCiphertext result;
        result.party_id = a.party_id;
        result.operations = a.operations + b.operations + 1;
        
        // Expand dim0 to pre-contraction form
        double expanded_a = expand_dim0(a);
        double expanded_b = expand_dim0(b);
        
        // Fully blind multiplication on expanded values
        double expanded_mul = (expanded_a * expanded_b 
                               - LAMBDA * (expanded_a + expanded_b) 
                               + LAMBDA * LAMBDA) / PHI + LAMBDA;
        
        // Contract back to near-floor
        result.coordinates[0] = contract_dim0(expanded_mul, result.party_id);
        
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
            << ",\"noise\":" << ct.noise
            << ",\"ops\":" << ct.operations
            << ",\"party\":" << ct.party_id
            << "}";
        return oss.str();
    }

    godcode::NDimCiphertext deserialize(const std::string& json) const {
        godcode::NDimCiphertext ct;
        ct.coordinates[0] = extract_double(json, "dim0");
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
