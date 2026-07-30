#pragma once
#include "../core/constants.h"
#include "../utils/safe_math.h"
#include "../utils/logger.h"
#include "../crypto/golden_fibonacci.h"
#include "../crypto/fractal_chaos.h"
#include <chrono>
#include <random>
#include <vector>
#include <algorithm>

struct EmergentRefreshController {
    double current_noise, emergent_threshold, chaos_state;
    int gates_since_refresh;
    int total_gates_at_refresh;
    
    EmergentRefreshController() : current_noise(0), emergent_threshold(0.5), 
                                   chaos_state(0.1), gates_since_refresh(0),
                                   total_gates_at_refresh(0) {}
    
    void update_threshold() {
        auto now = std::chrono::high_resolution_clock::now();
        auto nanos = std::chrono::duration_cast<std::chrono::nanoseconds>(now.time_since_epoch()).count();
        double t = SafeMath::fmod_safe(nanos * 1e-9 * PHI);
        t = FEIGENBAUM * t * (1.0 - t);
        double r = 3.99;
        for (int i = 0; i < 5; i++) t = r * t * (1.0 - t);
        
        emergent_threshold = 0.5 + t * 0.3;
        chaos_state = SafeMath::fmod_safe(chaos_state * 1.001 + t * 0.001);
        emergent_threshold = emergent_threshold * 0.8 + chaos_state * 0.2;
        
        if (emergent_threshold < 0.1 || std::isnan(emergent_threshold) || std::isinf(emergent_threshold)) 
            emergent_threshold = 0.4;
        if (emergent_threshold > 0.95) 
            emergent_threshold = 0.8;
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
        
        double safe_threshold = emergent_threshold;
        if (safe_threshold < 0.0 || safe_threshold > 1.0 || std::isnan(safe_threshold) || std::isinf(safe_threshold))
            safe_threshold = 0.5;
        
        bool noise_trigger = (gates_since_refresh > 15) && (current_noise > safe_threshold);
        bool safety_trigger = (gates_since_refresh > 50);
        
        if (noise_trigger || safety_trigger) {
            // Store gates_since BEFORE reset
            int gates_at_refresh = gates_since_refresh;
            total_gates_at_refresh = global_gate_count;
            
            Logger::info("  >>> Auto-refresh triggered at global gate " + std::to_string(global_gate_count));
            Logger::info("      Gates since last refresh: " + std::to_string(gates_at_refresh));
            Logger::info("      Noise: " + std::to_string(current_noise).substr(0,4) + 
                         " | Threshold: " + std::to_string(safe_threshold).substr(0,4) +
                         " | Chaos: " + std::to_string(chaos_state).substr(0,4));
            
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
    
    void init(double master_seed, int n_layers = 5, int d = 3, double iw = 0.75) {
        N = n_layers; depth = d; input_weight = iw;
        gf.init(master_seed, 50);
        Logger::info("FractalRefresh: N=" + std::to_string(N) + " depth=" + std::to_string(depth) + " auto-refresh=ON");
    }
    
    std::pair<double, double> refresh(double s_a, double s_b) {
        auto now = std::chrono::high_resolution_clock::now();
        auto nanos = std::chrono::duration_cast<std::chrono::nanoseconds>(now.time_since_epoch()).count();
        double t1 = SafeMath::fmod_safe(nanos * 1e-9 * PHI);
        double t2 = SafeMath::fmod_safe(nanos * 1e-9 * PHI * PHI);
        double t3 = SafeMath::fmod_safe(nanos * 1e-9 * PHI * PHI * PHI);
        t1 = FEIGENBAUM * t1 * (1.0 - t1); t2 = FEIGENBAUM * t2 * (1.0 - t2); t3 = FEIGENBAUM * t3 * (1.0 - t3);
        double r = 3.99;
        for (int i = 0; i < 8; i++) { t1 = r * t1 * (1.0 - t1); t2 = r * t2 * (1.0 - t2); t3 = r * t3 * (1.0 - t3); }
        double pre_delay = 0.00005 + t1 * 0.2, decrypt_delay = 0.00005 + t2 * 0.3, post_delay = 0.00005 + t3 * 0.2;
        volatile double es = t1 * PHI;
        auto pre = std::chrono::high_resolution_clock::now();
        while(std::chrono::duration<double>(std::chrono::high_resolution_clock::now()-pre).count()<pre_delay) es=r*es*(1.0-es);
        auto dec_start = std::chrono::high_resolution_clock::now();
        while(std::chrono::duration<double>(std::chrono::high_resolution_clock::now()-dec_start).count()<decrypt_delay) {
            es=r*es*(1.0-es); es=SafeMath::fmod_safe(es+t3*PSI);
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
        double np=sum_all/total*0.35+SafeMath::pow_safe(prod_all,1.0/total)*0.25+(total/harm_sum)*0.25+SafeMath::sqrt_safe(sum_sq/total)*0.15;
        double npsi=(max_v-min_v)*0.4+geom_sum/N*0.4+(1.0-total/harm_sum)*0.2;
        np=np*(1.0-input_weight)+s_a*input_weight;npsi=npsi*(1.0-input_weight)+s_b*input_weight;
        np=SafeMath::fmod_safe(np);npsi=SafeMath::fmod_safe(npsi);
        auto post=std::chrono::high_resolution_clock::now();
        while(std::chrono::duration<double>(std::chrono::high_resolution_clock::now()-post).count()<post_delay) es=r*es*(1.0-es);
        return {np,npsi};
    }
};
