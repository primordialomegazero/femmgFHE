#pragma once
#include "banach_engine.h"
#include <cmath>
#include <cstdint>
#include <string>
#include <sstream>
#include <iomanip>

constexpr double PHI      = banach::PHI;
constexpr double OCC      = banach::OCC;
constexpr double LAMBDA   = banach::LAMBDA;
constexpr int    DEPTH    = banach::DEPTH;
constexpr int    PARTIES  = banach::PARTIES;

class FEmmgFHE {
private:
    banach::NDimBanachEngine engine;
    int party_counter = 0;

public:
    FEmmgFHE() = default;

    banach::NDimCiphertext encrypt(int64_t m, int party = -1) {
        if(party < 0) party = (party_counter++) % PARTIES;
        return engine.encrypt(m, party);
    }

    int64_t decrypt(const banach::NDimCiphertext& ct) const {
        return engine.decrypt(ct);
    }

    banach::NDimCiphertext add(const banach::NDimCiphertext& a,
                                 const banach::NDimCiphertext& b) {
        banach::NDimCiphertext result;
        result.party_id = a.party_id;
        result.operations = a.operations + b.operations + 1;
        
        // Use cached expanded values for blind add
        long double ea = a.expanded_dim0;
        long double eb = b.expanded_dim0;
        result.expanded_dim0 = ea + eb - LAMBDA;
        
        // Self-referential re-contract
        result.noise = a.noise * OCC + b.noise * (1.0 - OCC);
        result.phi_state = a.phi_state * OCC + b.phi_state * (1.0 - OCC);
        engine.recontract_dim0(result);
        
        for(int d = 1; d < banach::DIMS; d++) {
            result.coordinates[d] = a.coordinates[d] * OCC 
                                  + b.coordinates[d] * (1.0 - OCC);
        }
        
        return result;
    }

    banach::NDimCiphertext multiply(const banach::NDimCiphertext& a,
                                      const banach::NDimCiphertext& b) {
        banach::NDimCiphertext result;
        result.party_id = a.party_id;
        result.operations = a.operations + b.operations + 1;
        
        long double ea = a.expanded_dim0;
        long double eb = b.expanded_dim0;
        result.expanded_dim0 = (ea * eb - LAMBDA * (ea + eb) 
                                + LAMBDA * LAMBDA) / PHI + LAMBDA;
        
        result.noise = (a.noise + b.noise) * OCC;
        result.phi_state = (a.phi_state + b.phi_state) * OCC;
        engine.recontract_dim0(result);
        
        for(int d = 1; d < banach::DIMS; d++) {
            result.coordinates[d] = a.coordinates[d] * OCC 
                                  + b.coordinates[d] * (1.0 - OCC);
        }
        
        return result;
    }

    bool verify_roundtrip(int64_t tv, int p = 0) { return engine.verify_roundtrip(tv, p); }
    bool verify_contraction(const banach::NDimCiphertext& ct) const { return engine.verify_contraction(ct); }
};

#include "fractal_fhe.h"
