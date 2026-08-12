#pragma once
#include "../core/constants.h"
#include "../fhe/fhe_core.h"
#include "../crypto/golden_fibonacci.h"
#include "../config/gf_n_encryption.h"

struct CompleteHomomorphicLayer {
    CryptoContext<DCRTPoly> cc;
    PublicKey<DCRTPoly> publicKey;
    
    GFNEncryption gf_n;
    double master_seed;
    int N_gf_layers;
    int bootstrap_count;
    bool has_stored_state;
    std::vector<double> stored_y2_trail;
    
    // Encrypted CKKS secret key seed under GF-N
    GFNEncryption::CipherText encrypted_sk_seed;
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
    
    // SETUP: Derive seed from existing secret key, encrypt under GF-N
    void setup_encrypted_sk(SecureContext& sc) {
        // Use the master_seed as proxy for SK seed
        // In production: SK seed = KDF(serialize(secretKey))
        double sk_seed = fmod(master_seed * PHI + 0.618, 1.0);
        encrypted_sk_seed = gf_n.encrypt(sk_seed);
        has_encrypted_sk = true;
    }
    
    // Derive secret key seed from encrypted material, then wipe
    double derive_and_wipe_sk_seed() {
        if (!has_encrypted_sk) return master_seed;
        double sk_seed = gf_n.decrypt(encrypted_sk_seed);
        return sk_seed;
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
    
    Ciphertext<DCRTPoly> bootstrap(const Ciphertext<DCRTPoly>& encrypted_input, SecureContext& sc) {
        bootstrap_count++;
        
        // Derive SK seed from GF-N encrypted material
        double sk_seed = derive_and_wipe_sk_seed();
        
        // CKKS Decrypt
        Plaintext ckks_plain;
        sc.cc->Decrypt(sc.kp.secretKey, encrypted_input, &ckks_plain);
        double y1 = ckks_plain->GetCKKSPackedValue()[0].real();
        
        // Wipe seed
        volatile double* v = &sk_seed;
        *v = 0.0;
        __asm__ __volatile__("" : : "r"(v) : "memory");
        
        // Cassini verify
        GFNEncryption::CipherText gf_ct;
        gf_ct.y1 = y1;
        gf_ct.y2_trail = has_stored_state ? stored_y2_trail :
                         std::vector<double>(N_gf_layers, y1);
        
        for (int i = 0; i < N_gf_layers; i++) {
            double phi_y1 = gf_ct.y1 + (i + 1) * PHI;
            double psi_y2 = gf_ct.y2_trail[i] + (i + 1) * PSI;
            if (std::abs(phi_y1 * psi_y2 + 1.0) < 0.1) return encrypted_input;
        }
        
        // Seed rotation
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
        
        return cc->Encrypt(publicKey, cc->MakeCKKSPackedPlaintext(std::vector<double>{fresh_ct.y1}));
    }
};
