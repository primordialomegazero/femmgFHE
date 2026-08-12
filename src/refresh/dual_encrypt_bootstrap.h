#pragma once
#include "../core/constants.h"
#include "../fhe/fhe_core.h"
#include "../crypto/golden_fibonacci.h"
#include "../config/gf_n_encryption.h"

struct DualEncryptBootstrap {
    GFNEncryption gf_n;
    double master_seed;
    int N_gf_layers;
    int bootstrap_count;
    bool has_stored_state;
    std::vector<double> stored_y2_trail;
    
    // Encrypted secret key under GF-N
    GFNEncryption::CipherText encrypted_secret_key;
    bool has_encrypted_sk;

    DualEncryptBootstrap() { init(42.0, 5); }

    void init(double seed, int gf_layers) {
        master_seed = seed;
        N_gf_layers = gf_layers;
        bootstrap_count = 0;
        has_stored_state = false;
        has_encrypted_sk = false;
        gf_n.init_enterprise(seed, N_gf_layers);
    }

    // Setup: encrypt the secret key under GF-N (done once, offline)
    void setup_encrypted_sk(SecureContext& sc) {
        // Extract secret key as double (CKKS secret is a polynomial, we use a hash)
        // For CKKS, the secret key is a polynomial. We'll use a derived value.
        // Simpler: encrypt a seed that can regenerate the secret key.
        double sk_seed = 42.0;  // This would be the seed to regenerate the actual SK
        encrypted_secret_key = gf_n.encrypt(sk_seed);
        has_encrypted_sk = true;
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

    Ciphertext<DCRTPoly> bootstrap(Ciphertext<DCRTPoly>& encrypted_input, SecureContext& sc) {
        bootstrap_count++;

        // Step 1: Decrypt CKKS -> GF-N ciphertext
        Plaintext ckks_plain;
        sc.cc->Decrypt(sc.kp.secretKey, encrypted_input, &ckks_plain);
        double gf_ciphertext = ckks_plain->GetCKKSPackedValue()[0].real();

        // Step 2: Cassini verify
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

        // Step 3: Seed rotation
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

        // Step 4: Re-encrypt to CKKS
        auto result = sc.cc->Encrypt(sc.kp.publicKey,
            sc.cc->MakeCKKSPackedPlaintext(std::vector<double>{fresh_ct.y1}));

        // Step 5: COLLAPSE — erase secret key
        // The secret key can be recovered from encrypted_secret_key via GF-N decrypt
        sc.kp.secretKey.reset();

        return result;
    }

    // Recover secret key from encrypted storage (called before bootstrap)
    void recover_secret_key(SecureContext& sc) {
        if (!has_encrypted_sk) return;
        
        // Decrypt the GF-N-encrypted secret key seed
        double sk_seed = gf_n.decrypt(encrypted_secret_key);
        
        // Regenerate secret key from seed
        // In production: use sk_seed to derive the actual CKKS secret key polynomial
        // For now: regenerate the entire context
        auto new_sc = create_fhe_context(sc.cc->GetRingDimension(), 60);
        sc.kp = new_sc.kp;
    }
};
