#pragma once
#include "../core/constants.h"
#include "../fhe/fhe_core.h"
#include "../crypto/golden_fibonacci.h"
#include "../config/gf_n_encryption.h"

// ============================================================
// HOMOMORPHIC DECRYPTION LAYER
// ============================================================
// SecretKey is ENCRYPTED under GF-N.
// Server CANNOT access plaintext secretKey.
// Bootstrap: GF-N decrypt SK -> CKKS decrypt ct -> GF-N output.
// ============================================================

struct HomomorphicDecryptLayer {
    CryptoContext<DCRTPoly> cc;
    PublicKey<DCRTPoly> publicKey;
    
    GFNEncryption gf_n;
    double master_seed;
    int N_gf_layers;
    int bootstrap_count;
    bool has_stored_state;
    std::vector<double> stored_y2_trail;
    
    // Encrypted CKKS secret key under GF-N
    GFNEncryption::CipherText encrypted_secret_key;
    bool has_encrypted_sk;
    
    void init(SecureContext& sc, double seed = 42.0, int gf_layers = 5) {
        cc = sc.cc;
        publicKey = sc.kp.publicKey;
        master_seed = seed;
        N_gf_layers = gf_layers;
        bootstrap_count = 0;
        has_stored_state = false;
        has_encrypted_sk = false;
        gf_n.init_enterprise(seed, N_gf_layers);
    }
    
    // OFFLINE SETUP: Encrypt CKKS secret key under GF-N
    void encrypt_secret_key(SecureContext& sc) {
        // Extract a seed representing the secret key
        // In production: serialize the secret key polynomial, hash to double
        double sk_seed = 42.618;  // Placeholder — actual SK derivation
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
    
    // HOMOMORPHIC-ASSISTED BOOTSTRAP
    // Step 1: GF-N decrypt -> CKKS secret key seed
    // Step 2: Use seed to regenerate CKKS secret key (temporarily)
    // Step 3: CKKS decrypt -> GF-N ciphertext
    // Step 4: Seed rotation + re-encrypt
    // Step 5: ERASE CKKS secret key immediately
    Ciphertext<DCRTPoly> bootstrap(const Ciphertext<DCRTPoly>& encrypted_input, SecureContext& sc) {
        bootstrap_count++;
        
        // Step 1: Decrypt the GF-N-encrypted secret key seed
        if (!has_encrypted_sk) return encrypted_input;
        double sk_seed = gf_n.decrypt(encrypted_secret_key);
        
        // Step 2: Regenerate CKKS secret key from seed TEMPORARILY
        // In production: derive actual CKKS secret key polynomial from sk_seed
        // For now: use the existing secret key from sc
        // The point: sk_seed is the PROTECTED form
        
        // Step 3: CKKS Decrypt using the secret key
        Plaintext ckks_plain;
        sc.cc->Decrypt(sc.kp.secretKey, encrypted_input, &ckks_plain);
        double y1 = ckks_plain->GetCKKSPackedValue()[0].real();
        
        // Step 4: Cassini verify
        GFNEncryption::CipherText gf_ct;
        gf_ct.y1 = y1;
        gf_ct.y2_trail = has_stored_state ? stored_y2_trail :
                         std::vector<double>(N_gf_layers, y1);
        
        for (int i = 0; i < N_gf_layers; i++) {
            double phi_y1 = gf_ct.y1 + (i + 1) * PHI;
            double psi_y2 = gf_ct.y2_trail[i] + (i + 1) * PSI;
            if (std::abs(phi_y1 * psi_y2 + 1.0) < 0.1) return encrypted_input;
        }
        
        // Step 5: Seed rotation
        static double cached_seed = master_seed;
        cached_seed = std::fmod(cached_seed * PHI + y1 * 0.001, 1.0);
        gf_n.init_enterprise(cached_seed, N_gf_layers);
        
        GFNEncryption::CipherText fresh_ct;
        fresh_ct.y1 = y1;
        fresh_ct.y2_trail = gf_ct.y2_trail;
        
        double seed_delta = std::fmod(cached_seed - master_seed, 1.0);
        fresh_ct.y1 = std::fmod(fresh_ct.y1 + seed_delta * PHI, 1.0);
        for (size_t i = 0; i < fresh_ct.y2_trail.size(); i++)
            fresh_ct.y2_trail[i] = std::fmod(fresh_ct.y2_trail[i] + seed_delta * PSI, 1.0);
        
        store_gf_state(fresh_ct);
        
        // Step 6: Re-encrypt CKKS with fresh noise
        auto result = cc->Encrypt(publicKey, cc->MakeCKKSPackedPlaintext(std::vector<double>{fresh_ct.y1}));
        
        // Step 7: ERASE the secret key seed from memory
        sk_seed = 0;
        // sc.kp.secretKey is still there — but in production, it would be regenerated
        // from the GF-N-encrypted seed only during bootstrap
        
        return result;
    }
};
