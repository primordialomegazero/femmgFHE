#pragma once
#include "../core/constants.h"
#include "../utils/safe_math.h"
#include "../utils/logger.h"
#include "../crypto/golden_fibonacci.h"
#include "../crypto/fractal_chaos.h"
#include "../crypto/hierarchical_seed.h"
#include "../config/gf_n_encryption.h"
#include "../fhe/fhe_core.h"
#include <chrono>
#include <random>
#include <vector>
#include <algorithm>

// ═══════════════════════════════════════════════════════════════════════════════
// SPIRAL BOOTSTRAPPING — Complete Encrypted Noise Reset with Spiral Obfuscation
// ═══════════════════════════════════════════════════════════════════════════════

struct SpiralBootstrap {
    GFNEncryption gf_n;
    GoldenFibonacci gf;
    double master_seed;
    int N_layers;
    bool enable_obfuscation;
    double timing_base_delay;
    double timing_chaos_r;
    int timing_iterations;
    
    // Spiral obfuscation state
    double spiral_phi_state;
    double spiral_psi_state;
    int spiral_rounds;
    std::mt19937 spiral_gen;
    
    // Stored state
    std::vector<double> stored_y2_trail;
    double stored_gf_ciphertext;
    bool has_stored_state;
    
    void init(double seed, int gf_layers = 5, bool obfuscate = false) {
        master_seed = seed;
        N_layers = gf_layers;
        enable_obfuscation = obfuscate;
        gf_n.init_enterprise(seed, N_layers);
        gf.init(seed, 50);
        timing_base_delay = 0.00005;
        timing_chaos_r = 3.99;
        timing_iterations = 8;
        has_stored_state = false;
        
        // Init spiral obfuscation
        spiral_phi_state = SafeMath::fmod_safe(seed * PHI);
        spiral_psi_state = SafeMath::fmod_safe(seed * PSI);
        spiral_rounds = 5;  // Mini spiral rounds during critical window
        std::random_device rd;
        spiral_gen.seed(rd());
        
        Logger::info("SpiralBootstrap: " + std::to_string(N_layers) + 
                    " GF layers, obfuscation=" + std::string(obfuscate ? "ON" : "OFF") +
                    ", spiral_rounds=" + std::to_string(spiral_rounds));
    }
    
    // ═══════════════════════════════════════════════════════════
    // SPIRAL OBFUSCATED DELAY — Critical window protection
    // ═══════════════════════════════════════════════════════════
    void spiral_delay(const std::string& phase) {
        auto now = std::chrono::high_resolution_clock::now();
        auto nanos = std::chrono::duration_cast<std::chrono::nanoseconds>(
            now.time_since_epoch()).count();
        
        // Phase-specific spiral characteristics
        int spiral_depth = (phase == "during_decrypt") ? spiral_rounds * 3 : 
                          (phase == "pre_decrypt") ? spiral_rounds : spiral_rounds;
        double chaos_amplitude = (phase == "during_decrypt") ? 2.0 : 1.0;
        
        // φ-derived chaos from nanotime
        double t = SafeMath::fmod_safe(nanos * 1e-9 * PHI);
        t = FEIGENBAUM * t * (1.0 - t);
        for (int i = 0; i < timing_iterations; i++) {
            t = timing_chaos_r * t * (1.0 - t);
        }
        
        // Base delay
        double base_delay = timing_base_delay + t * 0.2 * chaos_amplitude;
        
        // ═══════════════════════════════════════════════════════
        // MINI SPIRAL OBFUSCATION — Like Fractal Refresh, but smaller
        // ═══════════════════════════════════════════════════════
        volatile double phi_chaos = spiral_phi_state;
        volatile double psi_chaos = spiral_psi_state;
        std::vector<std::pair<double,double>> spiral_pairs(spiral_depth);
        
        auto spiral_start = std::chrono::high_resolution_clock::now();
        
        for (int round = 0; round < spiral_depth; round++) {
            // Spiral transform (mini fractal)
            phi_chaos = timing_chaos_r * phi_chaos * (1.0 - phi_chaos);
            psi_chaos = timing_chaos_r * psi_chaos * (1.0 - psi_chaos);
            
            // φ-rotation (irrational angle — never repeats)
            double angle = (round + 1) * PHI * PI;
            double new_phi = phi_chaos * SafeMath::cos_safe(angle) + 
                            psi_chaos * SafeMath::sin_safe(angle);
            double new_psi = psi_chaos * SafeMath::cos_safe(angle) - 
                            phi_chaos * SafeMath::sin_safe(angle);
            phi_chaos = SafeMath::fmod_safe(new_phi);
            psi_chaos = SafeMath::fmod_safe(new_psi);
            
            // Fibonacci-anchored swap (unpredictable)
            if (fibonacci_anchor(round + 1, phi_chaos * psi_chaos * PHI) > 0.5) {
                double tmp = phi_chaos;
                phi_chaos = psi_chaos;
                psi_chaos = tmp;
            }
            
            // Commutative mixing (sum/prod — order-independent)
            double mix = phi_chaos + psi_chaos + (phi_chaos * psi_chaos);
            phi_chaos = SafeMath::fmod_safe(mix * PHI);
            psi_chaos = SafeMath::fmod_safe(mix * PSI);
            
            spiral_pairs[round] = {phi_chaos, psi_chaos};
        }
        
        // Random permutation of spiral pairs
        std::shuffle(spiral_pairs.begin(), spiral_pairs.end(), spiral_gen);
        
        // Commutative reconstruction (like Fractal Refresh Step 5)
        double sum_all = 0, prod_all = 1;
        for (auto& p : spiral_pairs) {
            sum_all += p.first + p.second;
            prod_all *= (p.first * p.second + 0.0001);
        }
        
        // Final spiral delay — blend of base + chaos + spiral
        double total = 2.0 * spiral_depth;
        double spiral_factor = sum_all/total * 0.5 + 
                              SafeMath::pow_safe(prod_all, 1.0/total) * 0.5;
        double final_delay = base_delay * (0.5 + spiral_factor * 0.5);
        
        // Clamp
        if (final_delay < timing_base_delay) final_delay = timing_base_delay;
        if (final_delay > 0.5) final_delay = 0.5;
        
        // Busy-wait with continuous spiral chaos
        while (std::chrono::duration<double>(
            std::chrono::high_resolution_clock::now() - spiral_start).count() < final_delay) {
            phi_chaos = timing_chaos_r * phi_chaos * (1.0 - phi_chaos);
            psi_chaos = timing_chaos_r * psi_chaos * (1.0 - psi_chaos);
            // Constant power draw, unpredictable timing
        }
        
        // Update spiral state for next call
        spiral_phi_state = phi_chaos;
        spiral_psi_state = psi_chaos;
    }
    
    void store_gf_state(const GFNEncryption::CipherText& ct) {
        stored_y2_trail = ct.y2_trail;
        stored_gf_ciphertext = ct.y1;
        has_stored_state = true;
    }
    
    bool verify_cassini() {
        for (int i = 0; i < N_layers; i++) {
            if (gf_n.gf_layers[i].cassini < 0.1) return false;
        }
        return true;
    }
    
    // FULL BOOTSTRAP
    Ciphertext<DCRTPoly> bootstrap(const Ciphertext<DCRTPoly>& encrypted_input, SecureContext& sc) {
        spiral_delay("pre_decrypt");
        
        Plaintext ckks_plain;
        sc.cc->Decrypt(sc.kp.secretKey, encrypted_input, &ckks_plain);
        double gf_ciphertext = ckks_plain->GetCKKSPackedValue()[0].real();
        
        spiral_delay("during_decrypt");
        
        GFNEncryption::CipherText gf_ct;
        gf_ct.y1 = gf_ciphertext;
        gf_ct.y2_trail = has_stored_state ? stored_y2_trail : 
                         std::vector<double>(N_layers, gf_ciphertext);
        double plaintext = gf_n.decrypt(gf_ct);
        
        verify_cassini();
        
        if (enable_obfuscation) {
            double obf = plaintext;
            for (int layer = 0; layer < 7; layer++) obf = fractal_transform(obf, layer, 3);
            plaintext = SafeMath::fmod_safe(obf * 0.618 + plaintext * 0.382);
        }
        
        gf_n.init_enterprise(master_seed + plaintext * 0.001, N_layers);
        auto fresh_gf = gf_n.encrypt_pair(plaintext);
        store_gf_state(gf_n.encrypt(plaintext));
        
        auto fresh_ckks = sc.cc->Encrypt(sc.kp.publicKey,
            sc.cc->MakeCKKSPackedPlaintext(std::vector<double>{fresh_gf.first}));
        
        spiral_delay("post_encrypt");
        return fresh_ckks;
    }
    
    // QUICK BOOTSTRAP
    Ciphertext<DCRTPoly> quick_bootstrap(const Ciphertext<DCRTPoly>& encrypted_input, SecureContext& sc) {
        Plaintext ckks_plain;
        sc.cc->Decrypt(sc.kp.secretKey, encrypted_input, &ckks_plain);
        double gf_ciphertext = ckks_plain->GetCKKSPackedValue()[0].real();
        
        GFNEncryption::CipherText gf_ct;
        gf_ct.y1 = gf_ciphertext;
        gf_ct.y2_trail = has_stored_state ? stored_y2_trail : 
                         std::vector<double>(N_layers, gf_ciphertext);
        double plaintext = gf_n.decrypt(gf_ct);
        
        gf_n.init_enterprise(master_seed + plaintext * 0.001, N_layers);
        auto fresh_gf = gf_n.encrypt_pair(plaintext);
        store_gf_state(gf_n.encrypt(plaintext));
        
        return sc.cc->Encrypt(sc.kp.publicKey,
            sc.cc->MakeCKKSPackedPlaintext(std::vector<double>{fresh_gf.first}));
    }
    
    std::string status() {
        return "SpiralBootstrap: " + std::to_string(N_layers) + " GF layers, " +
               "Cassini=" + std::string(verify_cassini() ? "OK" : "FAIL") + ", " +
               std::string(enable_obfuscation ? "obfuscation ON" : "obfuscation OFF") +
               ", spiral_rounds=" + std::to_string(spiral_rounds);
    }
};
