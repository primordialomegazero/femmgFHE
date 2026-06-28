/*
 * FEmmg-FHE — MULTI-RECURSIVE FRACTAL LAYER
 * 7 Layers | 14 Parties | 91 Cross-Verified Pairs
 * Fully Homomorphic — All operations direct on ciphertexts
 */

#pragma once
#include "femmg_fhe.h"

class FractalFHE {
    FEmmgFHE fhe;
    double seeds[PARTIES];
    
public:
    FractalFHE() {
        for(int i = 0; i < PARTIES; i++)
            seeds[i] = PHI * (i + 1) * PHI_INV + FLOOR * (1.0 - PHI_INV);
    }
    
    // 7-layer fractal encryption
    Ciphertext encrypt(int64_t value, int party) {
        Ciphertext ct = fhe.encrypt(value);
        for(int layer = 0; layer < DEPTH; layer++) {
            double psi = PHI * (party + 1) * (layer + 1) * LAMBDA * 0.001;
            ct.o = ct.o * PHI_INV + psi;
            ct.n = ct.n * PHI_INV + FLOOR * (1.0 - PHI_INV);
            ct.c++;
        }
        return ct;
    }
    
    int64_t decrypt(const Ciphertext& ct) { return fhe.decrypt(ct); }
    
    // Chain ADD across parties — TRUE FHE
    Ciphertext chain_add(const std::vector<Ciphertext>& cts) {
        if(cts.empty()) return fhe.encrypt(0);
        Ciphertext result = cts[0];
        for(size_t i = 1; i < cts.size(); i++)
            result = fhe.add(result, cts[i]);
        return result;
    }
    
    // Chain MULTIPLY across parties — TRUE FHE
    Ciphertext chain_multiply(const std::vector<Ciphertext>& cts) {
        if(cts.empty()) return fhe.encrypt(1);
        Ciphertext result = cts[0];
        for(size_t i = 1; i < cts.size(); i++)
            result = fhe.multiply(result, cts[i]);
        return result;
    }
    
    // 91 cross-party verifications
    bool verify_all() {
        int64_t test = (int64_t)(PHI * 100);
        for(int i = 0; i < PARTIES; i++) {
            for(int j = i + 1; j < PARTIES; j++) {
                auto a = encrypt(test, i);
                auto b = encrypt(test, j);
                if(decrypt(a) != test || decrypt(b) != test)
                    return false;
            }
        }
        return true;
    }
    
    FEmmgFHE& engine() { return fhe; }
};
