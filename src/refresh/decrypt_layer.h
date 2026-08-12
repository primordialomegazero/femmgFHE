#pragma once
#include "../core/constants.h"
#include "../fhe/fhe_core.h"
#include "../crypto/golden_fibonacci.h"
#include "../config/gf_n_encryption.h"

// ============================================================
// DEDICATED DECRYPTION LAYER
// ============================================================
// Holds CKKS secretKey internally.
// Decrypts CKKS -> GF-N ciphertext (NOT original plaintext).
// Even if compromised, only GF-N ciphertext is exposed.
// ============================================================

struct DecryptLayer {
    PrivateKey<DCRTPoly> secretKey;  // CKKS secret key (SECURE)
    CryptoContext<DCRTPoly> cc;      // CKKS context
    PublicKey<DCRTPoly> publicKey;   // For re-encryption
    
    GFNEncryption gf_n;              // GF-N engine
    double master_seed;
    int N_gf_layers;
    int bootstrap_count;
    bool has_stored_state;
    std::vector<double> stored_y2_trail;
    
    void init(SecureContext& sc, double seed = 42.0, int gf_layers = 5) {
        secretKey = sc.kp.secretKey;
        cc = sc.cc;
        publicKey = sc.kp.publicKey;
        master_seed = seed;
        N_gf_layers = gf_layers;
        bootstrap_count = 0;
        has_stored_state = false;
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
    
    // DECRYPT CKKS -> GF-N CIPHERTEXT (NOT original plaintext!)
    // Returns: Enc_CKKS(new_y1) with fresh noise budget
    Ciphertext<DCRTPoly> decrypt_to_gf(const Ciphertext<DCRTPoly>& encrypted_input) {
        bootstrap_count++;
        
        // Step 1: CKKS Decrypt -> y1 (GF-N ciphertext, NOT original data)
        Plaintext ckks_plain;
        cc->Decrypt(secretKey, encrypted_input, &ckks_plain);
        double y1 = ckks_plain->GetCKKSPackedValue()[0].real();
        
        // Step 2: Cassini verify
        GFNEncryption::CipherText gf_ct;
        gf_ct.y1 = y1;
        gf_ct.y2_trail = has_stored_state ? stored_y2_trail :
                         std::vector<double>(N_gf_layers, y1);
        
        for (int i = 0; i < N_gf_layers; i++) {
            double phi_y1 = gf_ct.y1 + (i + 1) * PHI;
            double psi_y2 = gf_ct.y2_trail[i] + (i + 1) * PSI;
            if (std::abs(phi_y1 * psi_y2 + 1.0) < 0.1) return encrypted_input;
        }
        
        // Step 3: Seed rotation
        static double cached_seed = master_seed;
        cached_seed = std::fmod(cached_seed * PHI + y1 * 0.001, 1.0);
        gf_n.init_enterprise(cached_seed, N_gf_layers);
        
        // Step 4: Re-encrypt with fresh noise budget B0
        GFNEncryption::CipherText fresh_ct;
        fresh_ct.y1 = y1;
        fresh_ct.y2_trail = gf_ct.y2_trail;
        
        double seed_delta = std::fmod(cached_seed - master_seed, 1.0);
        fresh_ct.y1 = std::fmod(fresh_ct.y1 + seed_delta * PHI, 1.0);
        for (size_t i = 0; i < fresh_ct.y2_trail.size(); i++)
            fresh_ct.y2_trail[i] = std::fmod(fresh_ct.y2_trail[i] + seed_delta * PSI, 1.0);
        
        store_gf_state(fresh_ct);
        
        // Return fresh CKKS encryption
        return cc->Encrypt(publicKey, cc->MakeCKKSPackedPlaintext(std::vector<double>{fresh_ct.y1}));
    }
};
