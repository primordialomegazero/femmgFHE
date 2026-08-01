#pragma once
#include "../core/constants.h"
#include "../utils/safe_math.h"
#include "../utils/logger.h"
#include "../crypto/golden_fibonacci.h"
#include "../crypto/fractal_chaos.h"
#include "../crypto/hierarchical_seed.h"
#include "../config/gf_n_encryption.h"
#include "../config/system_config.h"
#include "../fhe/fhe_core.h"
#include <chrono>
#include <random>
#include <vector>
#include <algorithm>

// ═══════════════════════════════════════════════════════════════════════════════
// SPIRAL BOOTSTRAP — All parameters N-configurable, φ-anchored
// ═══════════════════════════════════════════════════════════════════════════════

struct SpiralBootstrap {
    GFNEncryption gf_n;
    GoldenFibonacci gf;
    double master_seed;
    
    // All configurable via SystemConfig or direct set
    int N_gf_layers;           // GF-N layers (was: hardcoded 5)
    int N_spiral_rounds;       // Spiral obfuscation rounds (was: 5)
    int N_spiral_depth;        // Spiral depth during critical window (was: 15)
    int N_timing_iterations;   // Chaos iterations for timing (was: 8)
    int N_obfuscation_layers;  // Fractal obfuscation layers (was: 7)
    double N_timing_base_delay;// Base delay for emergent timing (was: 0.00005)
    double N_timing_chaos_r;   // Chaos parameter for delays (was: 3.99)
    double N_obfuscation_blend; // Blend weight (was: 0.618)
    bool enable_obfuscation;
    
    // Spiral state
    double spiral_phi_state;
    double spiral_psi_state;
    std::mt19937 spiral_gen;
    
    // Stored state
    std::vector<double> stored_y2_trail;
    double stored_gf_ciphertext;
    bool has_stored_state;
    
    // ═══════════════════════════════════════════════════════════
    // INIT — All parameters φ-anchored, N-configurable
    // ═══════════════════════════════════════════════════════════
    void init(double seed, int gf_layers = 5, bool obfuscate = false) {
        master_seed = seed;
        N_gf_layers = gf_layers;
        enable_obfuscation = obfuscate;
        
        // φ-ANCHORED DEFAULTS (Fibonacci-scaled)
        N_spiral_rounds = fibonacci(5);       // F(5) = 8
        N_spiral_depth = fibonacci(6);        // F(6) = 13 (critical window)
        N_timing_iterations = fibonacci(4);   // F(4) = 5
        N_obfuscation_layers = fibonacci(5);  // F(5) = 8
        N_timing_base_delay = 0.00005;
        N_timing_chaos_r = 3.99;              // Deep chaos (Lyapunov > 0)
        N_obfuscation_blend = PHI / (PHI + 1.0);  // 0.6180339...
        
        gf_n.init_enterprise(seed, N_gf_layers);
        gf.init(seed, N_gf_layers * 10);  // φ-scaled from N_gf_layers
        has_stored_state = false;
        
        spiral_phi_state = SafeMath::fmod_safe(seed * PHI);
        spiral_psi_state = SafeMath::fmod_safe(seed * PSI);
        std::random_device rd;
        spiral_gen.seed(rd());
        
        Logger::info("SpiralBootstrap: " + std::to_string(N_gf_layers) + 
                    " GF layers, spiral=" + std::to_string(N_spiral_rounds) + "/" + std::to_string(N_spiral_depth) +
                    ", obfuscation=" + std::string(obfuscate ? "ON" : "OFF"));
    }
    
    // Init from SystemConfig (all N's from config)
    void init_from_config(double seed, const SystemConfig& cfg) {
        master_seed = seed;
        N_gf_layers = cfg.N_fne_layers;
        enable_obfuscation = false;  // Config-driven
        
        // φ-scaled from config
        N_spiral_rounds = fibonacci(cfg.N_chaos_iterations);
        N_spiral_depth = N_spiral_rounds * 3;  // 3x during critical window
        N_timing_iterations = cfg.N_chaos_iterations;
        N_obfuscation_layers = cfg.N_fractal_depth;
        N_timing_base_delay = 0.00005;
        N_timing_chaos_r = cfg.N_chaos_r_max;
        N_obfuscation_blend = cfg.N_refresh_weight;
        
        gf_n.init_enterprise(seed, N_gf_layers);
        gf.init(seed, N_gf_layers * 10);  // φ-scaled from N_gf_layers
        has_stored_state = false;
        
        spiral_phi_state = SafeMath::fmod_safe(seed * PHI);
        spiral_psi_state = SafeMath::fmod_safe(seed * PSI);
        std::random_device rd;
        spiral_gen.seed(rd());
        
        Logger::info("SpiralBootstrap: " + std::to_string(N_gf_layers) + 
                    " GF layers (from config)");
    }
    
    static int fibonacci(int n) {
        if (n <= 0) return 1; if (n == 1) return 2;
        int a = 1, b = 2;
        for (int i = 2; i <= n; i++) { int c = a + b; a = b; b = c; }
        return b;
    }
    
    // ═══════════════════════════════════════════════════════════
    // SPIRAL DELAY — Configurable rounds, φ-anchored timing
    // ═══════════════════════════════════════════════════════════
    void spiral_delay(const std::string& phase) {
        auto now = std::chrono::high_resolution_clock::now();
        auto nanos = std::chrono::duration_cast<std::chrono::nanoseconds>(
            now.time_since_epoch()).count();
        
        // Phase-specific configuration
        int rounds = (phase == "during_decrypt") ? N_spiral_depth : N_spiral_rounds;
        double chaos_amplitude = (phase == "during_decrypt") ? 2.0 : 1.0;
        
        // φ-derived chaos
        double t = SafeMath::fmod_safe(nanos * 1e-9 * PHI);
        t = FEIGENBAUM * t * (1.0 - t);
        for (int i = 0; i < N_timing_iterations; i++) {
            t = N_timing_chaos_r * t * (1.0 - t);
        }
        
        double base_delay = N_timing_base_delay + t * 0.2 * chaos_amplitude;
        
        // Mini spiral obfuscation
        volatile double phi_chaos = spiral_phi_state;
        volatile double psi_chaos = spiral_psi_state;
        std::vector<std::pair<double,double>> spiral_pairs(rounds);
        
        auto spiral_start = std::chrono::high_resolution_clock::now();
        
        for (int round = 0; round < rounds; round++) {
            phi_chaos = N_timing_chaos_r * phi_chaos * (1.0 - phi_chaos);
            psi_chaos = N_timing_chaos_r * psi_chaos * (1.0 - psi_chaos);
            
            double angle = (round + 1) * PHI * PI;
            double new_phi = phi_chaos * SafeMath::cos_safe(angle) + 
                            psi_chaos * SafeMath::sin_safe(angle);
            double new_psi = psi_chaos * SafeMath::cos_safe(angle) - 
                            phi_chaos * SafeMath::sin_safe(angle);
            phi_chaos = SafeMath::fmod_safe(new_phi);
            psi_chaos = SafeMath::fmod_safe(new_psi);
            
            if (fibonacci_anchor(round + 1, phi_chaos * psi_chaos * PHI) > 0.5) {
                double tmp = phi_chaos; phi_chaos = psi_chaos; psi_chaos = tmp;
            }
            
            double mix = phi_chaos + psi_chaos + (phi_chaos * psi_chaos);
            phi_chaos = SafeMath::fmod_safe(mix * PHI);
            psi_chaos = SafeMath::fmod_safe(mix * PSI);
            
            spiral_pairs[round] = {phi_chaos, psi_chaos};
        }
        
        std::shuffle(spiral_pairs.begin(), spiral_pairs.end(), spiral_gen);
        
        double sum_all = 0, prod_all = 1;
        for (auto& p : spiral_pairs) { sum_all += p.first + p.second; prod_all *= (p.first * p.second + 0.0001); }
        
        double total = 2.0 * rounds;
        double spiral_factor = sum_all/total * 0.5 + SafeMath::pow_safe(prod_all, 1.0/total) * 0.5;
        double final_delay = base_delay * (0.5 + spiral_factor * 0.5);
        
        if (final_delay < N_timing_base_delay) final_delay = N_timing_base_delay;
        if (final_delay > 0.5) final_delay = 0.5;
        
        while (std::chrono::duration<double>(
            std::chrono::high_resolution_clock::now() - spiral_start).count() < final_delay) {
            phi_chaos = N_timing_chaos_r * phi_chaos * (1.0 - phi_chaos);
            psi_chaos = N_timing_chaos_r * psi_chaos * (1.0 - psi_chaos);
        }
        
        spiral_phi_state = phi_chaos;
        spiral_psi_state = psi_chaos;
    }
    
    void store_gf_state(const GFNEncryption::CipherText& ct) {
        stored_y2_trail = ct.y2_trail;
        stored_gf_ciphertext = ct.y1;
        has_stored_state = true;
    }
    
    bool verify_cassini() {
        for (int i = 0; i < N_gf_layers; i++)
            if (gf_n.gf_layers[i].cassini < 0.1) return false;
        return true;
    }
    
    // Full bootstrap with configurable obfuscation
    Ciphertext<DCRTPoly> bootstrap(const Ciphertext<DCRTPoly>& encrypted_input, SecureContext& sc) {
        spiral_delay("pre_decrypt");
        
        Plaintext ckks_plain;
        sc.cc->Decrypt(sc.kp.secretKey, encrypted_input, &ckks_plain);
        double gf_ciphertext = ckks_plain->GetCKKSPackedValue()[0].real();
        
        spiral_delay("during_decrypt");
        
        GFNEncryption::CipherText gf_ct;
        gf_ct.y1 = gf_ciphertext;
        gf_ct.y2_trail = has_stored_state ? stored_y2_trail : std::vector<double>(N_gf_layers, gf_ciphertext);
        double plaintext = gf_n.decrypt(gf_ct);
        verify_cassini();
        
        if (enable_obfuscation) {
            double obf = plaintext;
            for (int layer = 0; layer < N_obfuscation_layers; layer++)
                obf = fractal_transform(obf, layer, 3);
            plaintext = SafeMath::fmod_safe(obf * N_obfuscation_blend + plaintext * (1.0 - N_obfuscation_blend));
        }
        
        gf_n.init_enterprise(master_seed + plaintext * 0.001, N_gf_layers);
        auto fresh_gf = gf_n.encrypt_pair(plaintext);
        store_gf_state(gf_n.encrypt(plaintext));
        
        auto fresh_ckks = sc.cc->Encrypt(sc.kp.publicKey,
            sc.cc->MakeCKKSPackedPlaintext(std::vector<double>{fresh_gf.first}));
        
        spiral_delay("post_encrypt");
        return fresh_ckks;
    }
    
    Ciphertext<DCRTPoly> quick_bootstrap(const Ciphertext<DCRTPoly>& encrypted_input, SecureContext& sc) {
        Plaintext ckks_plain;
        sc.cc->Decrypt(sc.kp.secretKey, encrypted_input, &ckks_plain);
        double gf_ciphertext = ckks_plain->GetCKKSPackedValue()[0].real();
        
        GFNEncryption::CipherText gf_ct;
        gf_ct.y1 = gf_ciphertext;
        gf_ct.y2_trail = has_stored_state ? stored_y2_trail : std::vector<double>(N_gf_layers, gf_ciphertext);
        double plaintext = gf_n.decrypt(gf_ct);
        
        gf_n.init_enterprise(master_seed + plaintext * 0.001, N_gf_layers);
        auto fresh_gf = gf_n.encrypt_pair(plaintext);
        store_gf_state(gf_n.encrypt(plaintext));
        
        return sc.cc->Encrypt(sc.kp.publicKey,
            sc.cc->MakeCKKSPackedPlaintext(std::vector<double>{fresh_gf.first}));
    }
    
    std::string status() {
        return "SpiralBootstrap: " + std::to_string(N_gf_layers) + " GF layers, " +
               "spiral=" + std::to_string(N_spiral_rounds) + "/" + std::to_string(N_spiral_depth) + ", " +
               "Cassini=" + std::string(verify_cassini() ? "OK" : "FAIL") + ", " +
               "obfuscation=" + std::string(enable_obfuscation ? "ON" : "OFF");
    }
};
