#pragma once
#include "../core/constants.h"
#include "../utils/safe_math.h"
#include "../utils/logger.h"
#include "../crypto/golden_fibonacci.h"
#include "../crypto/fractal_chaos.h"
#include "../config/system_config.h"
#include <chrono>
#include <random>
#include <vector>
#include <algorithm>

struct EmergentRefreshController {
    double current_noise, emergent_threshold, chaos_state;
    int gates_since_refresh, total_gates_at_refresh;
    
    // Configurable parameters (from SystemConfig)
    double N_emerge_base_threshold, N_emerge_chaos_r, N_emerge_chaos_iterations;
    double N_emerge_threshold_range, N_emerge_blend_weight, N_emerge_chaos_blend;
    double N_emerge_clamp_low, N_emerge_clamp_high, N_emerge_safe_threshold;
    int N_emerge_noise_gate_min, N_emerge_safety_gate_max;
    
    void init_from_config(const SystemConfig& cfg) {
        N_emerge_base_threshold = 0.5;
        N_emerge_chaos_r = cfg.N_chaos_r_max;  // 3.99 in chaos regime
        N_emerge_chaos_iterations = cfg.N_chaos_iterations;  // 5-10
        N_emerge_threshold_range = 0.3;
        N_emerge_blend_weight = cfg.N_refresh_weight;  // 0.618
        N_emerge_chaos_blend = 0.2;
        N_emerge_clamp_low = cfg.N_ks_threshold * 2;  // 0.1
        N_emerge_clamp_high = 0.8;
        N_emerge_safe_threshold = 0.5;
        N_emerge_noise_gate_min = 15;
        N_emerge_safety_gate_max = 50;
        
        current_noise = 0;
        emergent_threshold = N_emerge_base_threshold;
        chaos_state = 0.1;
        gates_since_refresh = 0;
        total_gates_at_refresh = 0;
    }

    EmergentRefreshController() : current_noise(0), emergent_threshold(0.5),
                                   chaos_state(0.1), gates_since_refresh(0),
                                   total_gates_at_refresh(0) {}

    void update_threshold() {
        auto now = std::chrono::high_resolution_clock::now();
        auto nanos = std::chrono::duration_cast<std::chrono::nanoseconds>(now.time_since_epoch()).count();
        double t = SafeMath::fmod_safe(nanos * 1e-9 * PHI);
        t = FEIGENBAUM * t * (1.0 - t);
        for (int i = 0; i < N_emerge_chaos_iterations; i++) 
            t = N_emerge_chaos_r * t * (1.0 - t);

        emergent_threshold = N_emerge_base_threshold + t * N_emerge_threshold_range;
        chaos_state = SafeMath::fmod_safe(chaos_state * 1.001 + t * 0.001);
        emergent_threshold = emergent_threshold * N_emerge_blend_weight + chaos_state * N_emerge_chaos_blend;

        if (emergent_threshold < N_emerge_clamp_low || std::isnan(emergent_threshold) || std::isinf(emergent_threshold))
            emergent_threshold = N_emerge_clamp_low * 4;
        if (emergent_threshold > N_emerge_clamp_high)
            emergent_threshold = N_emerge_clamp_high;
    }

    double estimate_noise(double phi, double psi) {
        double collapse_risk = (std::abs(phi) < 0.0001 && std::abs(psi) < 0.0001) ? 1.0 : 0.0;
        double separation = std::abs(phi - psi);
        double gate_pressure = (double)gates_since_refresh / 60.0;
        return gate_pressure * 0.7 + collapse_risk * 0.2 + (1.0 - separation) * 0.1;
    }

    bool should_refresh(double phi, double psi, int global_gate_count) {
        update_threshold();
        current_noise = estimate_noise(phi, psi);
        gates_since_refresh++;

        double safe_threshold = N_emerge_safe_threshold;
        if (safe_threshold < 0.0 || safe_threshold > 1.0 || std::isnan(safe_threshold) || std::isinf(safe_threshold))
            safe_threshold = 0.5;

        bool noise_trigger = (gates_since_refresh > N_emerge_noise_gate_min) && (current_noise > safe_threshold);
        bool safety_trigger = (gates_since_refresh > N_emerge_safety_gate_max);

        if (noise_trigger || safety_trigger) {
            int gates_at_refresh = gates_since_refresh;
            total_gates_at_refresh = global_gate_count;
            gates_since_refresh = 0;
            return true;
        }
        return false;
    }
};

struct FractalRefresh {
    int N, depth;
    double input_weight;
    GoldenFibonacci gf;
    EmergentRefreshController controller;
    
    // Configurable (from SystemConfig)
    double N_refresh_chaos_r, N_refresh_pre_delay_base, N_refresh_decrypt_delay_base;
    double N_refresh_post_delay_base, N_refresh_chaos_iterations;
    double N_refresh_commutative_w1, N_refresh_commutative_w2;
    double N_refresh_commutative_w3, N_refresh_commutative_w4;
    double N_refresh_npsi_w1, N_refresh_npsi_w2, N_refresh_npsi_w3;

    void init(double master_seed, int n_layers = 5, int d = 3, double iw = 0.75) {
        N = n_layers; depth = d; input_weight = iw;
        gf.init(master_seed, 50);
        
        // Default configurable values
        N_refresh_chaos_r = 3.99;
        N_refresh_pre_delay_base = 0.00005;
        N_refresh_decrypt_delay_base = 0.00005;
        N_refresh_post_delay_base = 0.00005;
        N_refresh_chaos_iterations = 8;
        N_refresh_commutative_w1 = 0.35;
        N_refresh_commutative_w2 = 0.25;
        N_refresh_commutative_w3 = 0.25;
        N_refresh_commutative_w4 = 0.15;
        N_refresh_npsi_w1 = 0.4;
        N_refresh_npsi_w2 = 0.4;
        N_refresh_npsi_w3 = 0.2;
        
        Logger::info("FractalRefresh: N=" + std::to_string(N) + " depth=" + std::to_string(depth) + " auto-refresh=ON");
    }
    
    void init_from_config(double master_seed, const SystemConfig& cfg) {
        init(master_seed, cfg.N_fractal_layers, cfg.N_fractal_depth, cfg.N_refresh_weight);
        N_refresh_chaos_r = cfg.N_chaos_r_max;
        N_refresh_chaos_iterations = cfg.N_chaos_iterations;
        controller.init_from_config(cfg);
    }

    std::pair<double, double> refresh(double s_a, double s_b) {
        auto now = std::chrono::high_resolution_clock::now();
        auto nanos = std::chrono::duration_cast<std::chrono::nanoseconds>(now.time_since_epoch()).count();
        double t1 = SafeMath::fmod_safe(nanos * 1e-9 * PHI);
        double t2 = SafeMath::fmod_safe(nanos * 1e-9 * PHI * PHI);
        double t3 = SafeMath::fmod_safe(nanos * 1e-9 * PHI * PHI * PHI);
        t1 = FEIGENBAUM * t1 * (1.0 - t1); t2 = FEIGENBAUM * t2 * (1.0 - t2); t3 = FEIGENBAUM * t3 * (1.0 - t3);
        for (int i = 0; i < N_refresh_chaos_iterations; i++) { 
            t1 = N_refresh_chaos_r * t1 * (1.0 - t1); 
            t2 = N_refresh_chaos_r * t2 * (1.0 - t2); 
            t3 = N_refresh_chaos_r * t3 * (1.0 - t3); 
        }
        double pre_delay = N_refresh_pre_delay_base + t1 * 0.2;
        double decrypt_delay = N_refresh_decrypt_delay_base + t2 * 0.3;
        double post_delay = N_refresh_post_delay_base + t3 * 0.2;
        volatile double es = t1 * PHI;
        auto pre = std::chrono::high_resolution_clock::now();
        while(std::chrono::duration<double>(std::chrono::high_resolution_clock::now()-pre).count()<pre_delay) es=N_refresh_chaos_r*es*(1.0-es);
        auto dec_start = std::chrono::high_resolution_clock::now();
        while(std::chrono::duration<double>(std::chrono::high_resolution_clock::now()-dec_start).count()<decrypt_delay) {
            es=N_refresh_chaos_r*es*(1.0-es); es=SafeMath::fmod_safe(es+t3*PSI);
        }
        std::random_device rd; std::mt19937 gen(rd());
        std::vector<std::pair<double,double>> pairs(N);
        for(int layer=0;layer<N;layer++){
            double fa=fractal_transform(s_a,layer,depth),fb=fractal_transform(s_b,layer,depth);
            pairs[layer]=(fibonacci_anchor(layer+1,s_a*s_b)>0.5)?std::make_pair(fa,fb):std::make_pair(fb,fa);
        }
        std::shuffle(pairs.begin(),pairs.end(),gen);
        double sum_all=0,prod_all=1,harm_sum=0,geom_sum=0,sum_sq=0,min_v=1,max_v=0;
        for(auto&p:pairs){
            double v1=p.first,v2=p.second;
            sum_all+=v1+v2;prod_all*=(v1*v2+0.0001);harm_sum+=1.0/(v1+0.001)+1.0/(v2+0.001);
            geom_sum+=SafeMath::sqrt_safe(v1*v2+0.0001);sum_sq+=v1*v1+v2*v2;
            min_v=std::min(min_v,std::min(v1,v2));max_v=std::max(max_v,std::max(v1,v2));
        }
        double total=2.0*N;
        double np = sum_all/total*N_refresh_commutative_w1 + 
                    SafeMath::pow_safe(prod_all,1.0/total)*N_refresh_commutative_w2 + 
                    (total/harm_sum)*N_refresh_commutative_w3 + 
                    SafeMath::sqrt_safe(sum_sq/total)*N_refresh_commutative_w4;
        double npsi = (max_v-min_v)*N_refresh_npsi_w1 + 
                      geom_sum/N*N_refresh_npsi_w2 + 
                      (1.0-total/harm_sum)*N_refresh_npsi_w3;
        np=np*(1.0-input_weight)+s_a*input_weight;npsi=npsi*(1.0-input_weight)+s_b*input_weight;
        np=SafeMath::fmod_safe(np);npsi=SafeMath::fmod_safe(npsi);
        auto post=std::chrono::high_resolution_clock::now();
        while(std::chrono::duration<double>(std::chrono::high_resolution_clock::now()-post).count()<post_delay) es=N_refresh_chaos_r*es*(1.0-es);
        return {np,npsi};
    }
};
