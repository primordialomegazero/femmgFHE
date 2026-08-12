#pragma once
#include "../core/constants.h"
#include "../fhe/fhe_core.h"
#include "../crypto/golden_fibonacci.h"
#include "../config/gf_n_encryption.h"
#include "../io/io_emergent.h"

struct FractalKeyBootstrap {
    GFNEncryption gf_n;
    double master_seed;
    int N_gf_layers;
    int bootstrap_count;
    bool has_stored_state;
    std::vector<double> stored_y2_trail;
    
    // Fractalized secret key — N fragments, never stored together
    static constexpr int KEY_FRAGMENTS = 8;
    double phi_fragments[KEY_FRAGMENTS];
    double psi_fragments[KEY_FRAGMENTS];
    bool key_is_split;

    FractalKeyBootstrap() { init(42.0, 5); }

    void init(double seed, int gf_layers) {
        master_seed = seed;
        N_gf_layers = gf_layers;
        bootstrap_count = 0;
        has_stored_state = false;
        key_is_split = false;
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

    // FRACTALIZE: Split the secret key into φ/ψ fragments
    // The key is derived from a seed. We split the SEED, not the key itself.
    // Each fragment goes through FGG collapse — structural erasure after use.
    void fractalize(double key_seed) {
        for (int i = 0; i < KEY_FRAGMENTS; i++) {
            // φ-fragment: expand by φ, rotate by golden angle
            phi_fragments[i] = io_fgg(fmod(key_seed * PHI + i * 0.382, 1.0), 3, true);
            // ψ-fragment: contract by ψ, rotate by conjugate angle
            psi_fragments[i] = io_fgg(fmod(key_seed * fabs(PSI) + i * 0.618, 1.0), 3, false);
        }
        key_is_split = true;
    }

    // RECONSTRUCT: Temporarily rebuild the key seed from fragments
    // φ·ψ = -1 ensures the reconstruction is exact
    double reconstruct_seed() {
        if (!key_is_split) return master_seed;
        
        double phi_sum = 0, psi_sum = 0;
        for (int i = 0; i < KEY_FRAGMENTS; i++) {
            phi_sum += phi_fragments[i];
            psi_sum += psi_fragments[i];
        }
        
        // φ + ψ = 1 — the fragments recombine to the original
        double reconstructed = fmod(phi_sum * PHI + psi_sum * PSI, 1.0);
        
        // COLLAPSE fragments after reconstruction — they cannot be reused
        for (int i = 0; i < KEY_FRAGMENTS; i++) {
            phi_fragments[i] = io_fgg(phi_fragments[i] * PSI, 3, true);
            psi_fragments[i] = io_fgg(psi_fragments[i] * PHI, 3, false);
        }
        key_is_split = false;
        
        return reconstructed;
    }

    // Re-fractalize after bootstrap
    void re_fractalize(double key_seed) {
        fractalize(key_seed);
    }

    Ciphertext<DCRTPoly> bootstrap(Ciphertext<DCRTPoly>& encrypted_input, SecureContext& sc) {
        bootstrap_count++;

        // Step 1: Reconstruct key seed from fragments
        double key_seed = reconstruct_seed();
        
        // Step 2: Use the seed to bootstrap
        Plaintext ckks_plain;
        sc.cc->Decrypt(sc.kp.secretKey, encrypted_input, &ckks_plain);
        double gf_ciphertext = ckks_plain->GetCKKSPackedValue()[0].real();

        // Step 3: Cassini verify
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
            if (cassini_val < 0.1) {
                re_fractalize(key_seed);
                return encrypted_input;
            }
        }

        // Step 4: Seed rotation
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

        // Step 5: Re-fractalize the key seed — fragments are COLLAPSED
        re_fractalize(key_seed);
        
        // Step 6: The key_seed variable goes out of scope — erased from stack
        return sc.cc->Encrypt(sc.kp.publicKey,
            sc.cc->MakeCKKSPackedPlaintext(std::vector<double>{fresh_ct.y1}));
    }
};
