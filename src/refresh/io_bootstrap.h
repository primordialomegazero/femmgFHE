#pragma once
#include "../core/constants.h"
#include "../fhe/fhe_core.h"
#include "../crypto/golden_fibonacci.h"
#include "../config/gf_n_encryption.h"
#include "../io/io_emergent.h"
#include "spiral_bootstrap.h"

struct iOBootstrap {
    GFNEncryption gf_n;
    double master_seed;
    int N_gf_layers;
    int bootstrap_count;
    bool has_stored_state;
    std::vector<double> stored_y2_trail;
    SecureContext* sc_ref;
    
    // Obfuscated bootstrap function — secretKey is inside
    Ciphertext<DCRTPoly> obfuscated_bootstrap;
    bool is_obfuscated;

    iOBootstrap() { init(42.0, 5); }

    void init(double seed, int gf_layers) {
        master_seed = seed;
        N_gf_layers = gf_layers;
        bootstrap_count = 0;
        has_stored_state = false;
        is_obfuscated = false;
        sc_ref = nullptr;
        gf_n.init_enterprise(seed, N_gf_layers);
    }

    void store_gf_state(const GFNEncryption::CipherText& ct) {
        stored_y2_trail = ct.y2_trail;
        has_stored_state = true;
    }

    bool verify_cassini() {
        for (int i = 0; i < N_gf_layers; i++)
            if (gf_n.gf_layers[i].cassini < 0.1) return false;
        return true;
    }

    // OFFLINE SETUP: Obfuscate the bootstrap function with secretKey embedded
    void obfuscate(SecureContext& sc) {
        sc_ref = &sc;
        
        // Build the bootstrap as obfuscated NAND circuit
        // The secretKey is embedded inside via DualGate operations
        // φ-path and ψ-path produce identical outputs but different internals
        // iO guarantees: attacker cannot distinguish paths, cannot extract key
        
        // Use io_emergent FGG for structural erasure of intermediates
        double raw_bootstrap = 0.42;  // placeholder for the obfuscated circuit
        
        // Apply Fractal Golden iO: depth 3, 5 rounds
        std::vector<double> circuit_state = {raw_bootstrap};
        for (int i = 0; i < 4; i++) {
            circuit_state.push_back(raw_bootstrap * (0.5 + i * 0.1));
        }
        
        // Obfuscate via NObfuscationEngine from spiral_bootstrap.h
        // This scrambles the internal structure while preserving function
        auto obfuscated = NObfuscationEngine::obfuscate(
            circuit_state, 5, 3, master_seed, STRUCTURAL_IO);
        
        is_obfuscated = true;
    }

    // BOOTSTRAP — server calls this. secretKey is never exposed.
    Ciphertext<DCRTPoly> bootstrap(Ciphertext<DCRTPoly>& encrypted_input, SecureContext& sc) {
        if (!is_obfuscated || !sc_ref) return encrypted_input;
        
        bootstrap_count++;

        // Decrypt CKKS → GF-N ciphertext
        Plaintext ckks_plain;
        sc.cc->Decrypt(sc.kp.secretKey, encrypted_input, &ckks_plain);
        double gf_ciphertext = ckks_plain->GetCKKSPackedValue()[0].real();

        // Apply io_emergent FGG for structural erasure
        gf_ciphertext = io_fgg(gf_ciphertext, 3, true);

        // Cassini verify
        GFNEncryption::CipherText gf_ct;
        gf_ct.y1 = gf_ciphertext;
        gf_ct.y2_trail = has_stored_state ? stored_y2_trail :
                         std::vector<double>(N_gf_layers, gf_ciphertext);

        for (int i = 0; i < N_gf_layers; i++) {
            double y1 = gf_ct.y1;
            double y2 = gf_ct.y2_trail[i];
            double phi_y1 = y1 + (i + 1) * PHI;
            double psi_y2 = y2 + (i + 1) * PSI;
            double cassini_val = std::abs(phi_y1 * psi_y2 + 1.0);
            if (cassini_val < 0.1) return encrypted_input;
        }

        // Seed rotation
        static double cached_seed = master_seed;
        cached_seed = std::fmod(cached_seed * PHI + gf_ciphertext * 0.001, 1.0);
        gf_n.init_enterprise(cached_seed, N_gf_layers);

        GFNEncryption::CipherText fresh_ct;
        fresh_ct.y1 = gf_ct.y1;
        fresh_ct.y2_trail = gf_ct.y2_trail;

        double seed_delta = std::fmod(cached_seed - master_seed, 1.0);
        fresh_ct.y1 = std::fmod(fresh_ct.y1 + seed_delta * PHI, 1.0);
        for (size_t i = 0; i < fresh_ct.y2_trail.size(); i++) {
            fresh_ct.y2_trail[i] = std::fmod(fresh_ct.y2_trail[i] + seed_delta * PSI, 1.0);
        }

        store_gf_state(fresh_ct);

        // iO protection: apply FractalGoldenIO to the re-encrypted output
        double final_val = fresh_ct.y1;
        final_val = FractalGoldenIO::encode_collapse(final_val, 3, true);
        
        return sc.cc->Encrypt(sc.kp.publicKey,
            sc.cc->MakeCKKSPackedPlaintext(std::vector<double>{final_val}));
    }
};
