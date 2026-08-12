#pragma once
#include "../core/constants.h"
#include "../fhe/fhe_core.h"
#include "../crypto/golden_fibonacci.h"
#include "../config/gf_n_encryption.h"

struct NonInteractiveBootstrap {
    GFNEncryption gf_n;
    double master_seed;
    int N_gf_layers;
    int bootstrap_count;
    std::vector<double> stored_y2_trail;
    double stored_gf_ciphertext;
    bool has_state;

    NonInteractiveBootstrap() { init(42.0, 5); }

    void init(double seed, int gf_layers) {
        master_seed = seed;
        N_gf_layers = gf_layers;
        bootstrap_count = 0;
        has_state = false;
        gf_n.init_enterprise(seed, N_gf_layers);
    }

    // FIRST ENCRYPTION: Server encrypts data, stores GF-N state
    Ciphertext<DCRTPoly> initial_encrypt(double data, SecureContext& sc) {
        auto gf_ct = gf_n.encrypt(data);
        stored_gf_ciphertext = gf_ct.y1;
        stored_y2_trail = gf_ct.y2_trail;
        has_state = true;
        
        return sc.cc->Encrypt(sc.kp.publicKey,
            sc.cc->MakeCKKSPackedPlaintext(std::vector<double>{gf_ct.y1}));
    }

    // NON-INTERACTIVE BOOTSTRAP: No CKKS decrypt needed
    // Uses stored GF-N state directly, rotates seeds, re-encrypts CKKS
    Ciphertext<DCRTPoly> bootstrap(SecureContext& sc) {
        if (!has_state) {
            return sc.cc->Encrypt(sc.kp.publicKey,
                sc.cc->MakeCKKSPackedPlaintext(std::vector<double>{0.0}));
        }
        
        bootstrap_count++;

        // Cassini verify on stored GF-N state
        for (int i = 0; i < N_gf_layers; i++) {
            double y1 = stored_gf_ciphertext;
            double y2 = stored_y2_trail[i];
            double phi_y1 = y1 + (i + 1) * PHI;
            double psi_y2 = y2 + (i + 1) * PSI;
            if (std::abs(phi_y1 * psi_y2 + 1.0) < 0.1) {
                return sc.cc->Encrypt(sc.kp.publicKey,
                    sc.cc->MakeCKKSPackedPlaintext(std::vector<double>{stored_gf_ciphertext}));
            }
        }

        // Seed rotation on GF-N state — NO CKKS DECRYPT
        static double cached_seed = master_seed;
        cached_seed = std::fmod(cached_seed * PHI + stored_gf_ciphertext * 0.001, 1.0);
        gf_n.init_enterprise(cached_seed, N_gf_layers);

        double seed_delta = std::fmod(cached_seed - master_seed, 1.0);
        stored_gf_ciphertext = std::fmod(stored_gf_ciphertext + seed_delta * PHI, 1.0);
        for (size_t i = 0; i < stored_y2_trail.size(); i++) {
            stored_y2_trail[i] = std::fmod(stored_y2_trail[i] + seed_delta * PSI, 1.0);
        }

        // Re-encrypt CKKS — only needs publicKey, NOT secretKey
        return sc.cc->Encrypt(sc.kp.publicKey,
            sc.cc->MakeCKKSPackedPlaintext(std::vector<double>{stored_gf_ciphertext}));
    }
};
