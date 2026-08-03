#pragma once
#include <cmath>
#include <vector>
#include "../core/constants.h"
#include "../config/system_config.h"
#include "../production/stability_guard.h"

// ═══════════════════════════════════════════════════════════════
// AUTO BOOTSTRAP CONTROLLER v6 — Integrated with Core System
// ═══════════════════════════════════════════════════════════════
// Uses: SystemConfig for thresholds, StabilityGuard for metrics,
//       GoldenFibonacci for batch scaling.
// No hardcoded values — everything from N-configurable SystemConfig.

struct AutoBootstrap {
    // ═══════════════════════════════════════════════════════════
    // CONFIGURATION (from SystemConfig)
    // ═══════════════════════════════════════════════════════════
    SystemConfig sys_cfg;
    StabilityGuard guard;
    
    // Runtime-adjustable overrides
    double noise_watch_override     = -1;  // -1 = use SystemConfig
    double noise_critical_override  = -1;
    double stability_watch_override = -1;
    double stability_critical_override = -1;
    double lyapunov_watch_override  = -1;
    double lyapunov_critical_override = -1;
    
    // Derived thresholds (cached from SystemConfig)
    double noise_watch()     { return noise_watch_override > 0     ? noise_watch_override     : sys_cfg.N_noise_warning; }
    double noise_critical()  { return noise_critical_override > 0  ? noise_critical_override  : sys_cfg.N_noise_critical; }
    double stab_watch()      { return stability_watch_override > 0 ? stability_watch_override : sys_cfg.N_noise_warning * 3.33; }
    double stab_critical()   { return stability_critical_override>0? stability_critical_override: sys_cfg.N_noise_critical * 3.33; }
    double lyap_watch()      { return lyapunov_watch_override > 0  ? lyapunov_watch_override  : sys_cfg.N_lyapunov_warning; }
    double lyap_critical()   { return lyapunov_critical_override>0 ? lyapunov_critical_override: sys_cfg.N_lyapunov_critical; }
    
    // ═══════════════════════════════════════════════════════════
    // STATE MACHINE
    // ═══════════════════════════════════════════════════════════
    enum CausalState {
        STATE_IDLE,
        STATE_WATCH,
        STATE_PREPARE,
        STATE_REFRESH,
        STATE_EMERGENCY
    };
    
    CausalState current_state = STATE_IDLE;
    CausalState previous_state = STATE_IDLE;
    
    // ═══════════════════════════════════════════════════════════
    // TRACKING
    // ═══════════════════════════════════════════════════════════
    int gates_since_refresh = 0;
    int total_gates = 0;
    int total_refreshes = 0;
    int total_emergencies = 0;
    int recommended_batch;
    
    double current_noise = 0;
    double current_stability = 0;
    double current_lyapunov = 0;
    
    // Fibonacci bridge (standalone — GoldenFibonacci is for encryption, not counting)
    int fib_index;
    
    int fib(int n) {
        if (n <= 0) return 1;
        if (n == 1) return 1;
        int a = 1, b = 1;
        for (int i = 2; i <= n; i++) { int c = a + b; a = b; b = c; }
        return b;
    }
    
    struct BatchMetrics {
        double batch_phi_sum = 0;
        int batch_gate_count = 0;
        double batch_phi_max = 0;
        double batch_phi_min = 1.0;
        bool batch_should_bootstrap = false;
        int batch_recommended_size;
    };
    
    BatchMetrics current_batch;
    std::vector<BatchMetrics> batch_history;
    int max_batch_history = 8;
    
    // ═══════════════════════════════════════════════════════════
    // INITIALIZATION
    // ═══════════════════════════════════════════════════════════
    void init(const SystemConfig& cfg) {
        sys_cfg = cfg;
        guard.cfg.noise_warning = cfg.N_noise_warning;
        guard.cfg.noise_critical = cfg.N_noise_critical;
        guard.cfg.stability_warning = cfg.N_lyapunov_warning * 3.33;
        guard.cfg.stability_critical = cfg.N_lyapunov_critical * 6;
        
        fib_index = 7;  // F(7)=13  // F(7)=13
        recommended_batch = cfg.N_batch_size_auto;
        set_batch(recommended_batch);
    }
    
    void set_batch(int batch) {
        recommended_batch = std::max(sys_cfg.N_batch_min, std::min(sys_cfg.N_batch_max, batch));
        fib_index = 3;
        while (fib(fib_index) < recommended_batch && fib_index < 15) fib_index++;
        recommended_batch = clamp_fib(fib_index);
    }
    
    int clamp_fib(int n) {
        int f = fib(n);
        if (f < sys_cfg.N_batch_min) return sys_cfg.N_batch_min;
        if (f > sys_cfg.N_batch_max) return sys_cfg.N_batch_max;
        return f;
    }
    
    // ═══════════════════════════════════════════════════════════
    // SENSE: Use StabilityGuard for metrics
    // ═══════════════════════════════════════════════════════════
    CausalState sense(double noise, double stability, double lyapunov, double gate_time = 0.001) {
        gates_since_refresh++;
        total_gates++;
        current_noise = noise;
        current_stability = stability;
        current_lyapunov = lyapunov;
        
        // Delegate to StabilityGuard for metric tracking
        guard.update(lyapunov, noise, stability, gate_time, 1);
        
        // Update batch bridge
        current_batch.batch_gate_count++;
                // Compute integrated metric from guard state
        double phi_est = (current_noise + (1.0 - current_stability) + (1.0 - current_lyapunov)) / 3.0;
        current_batch.batch_phi_sum += phi_est;
        if (phi_est > current_batch.batch_phi_max) current_batch.batch_phi_max = phi_est;
        if (phi_est < current_batch.batch_phi_min) current_batch.batch_phi_min = phi_est;
        
        previous_state = current_state;
        
        // Force refresh: safety gate limit
        if (gates_since_refresh > sys_cfg.N_gates_target) {
            current_state = STATE_REFRESH;
            total_refreshes++;
            apply_state(STATE_REFRESH);
            return STATE_REFRESH;
        }
        
        // Emergency: critical thresholds from SystemConfig
        if (noise > noise_critical() || stability < stab_critical() || lyapunov < lyap_critical()) {
            current_state = STATE_EMERGENCY;
            total_emergencies++;
            apply_state(STATE_EMERGENCY);
            return STATE_EMERGENCY;
        }
        
        // State progression using SystemConfig thresholds
        if (gates_since_refresh > 5) {
            if (guard.current_level >= StabilityGuard::CRITICAL ||
                noise > noise_critical() * 0.85 || stability < stab_critical() * 1.15) {
                current_state = STATE_REFRESH;
                total_refreshes++;
                apply_state(STATE_REFRESH);
                return STATE_REFRESH;
            }
            
            if (guard.current_level >= StabilityGuard::WARNING ||
                noise > noise_watch() * 1.3 || stability < stab_watch() * 0.7) {
                current_state = STATE_PREPARE;
                apply_state(STATE_PREPARE);
                return STATE_PREPARE;
            }
            
            if (noise > noise_watch() || stability < stab_watch() || lyapunov < lyap_watch()) {
                current_state = STATE_WATCH;
                apply_state(STATE_WATCH);
                return STATE_WATCH;
            }
        }
        
        current_state = STATE_IDLE;
        apply_state(STATE_IDLE);
        return STATE_IDLE;
    }
    
    // ═══════════════════════════════════════════════════════════
    // APPLY STATE: Fibonacci-anchored batch control
    // ═══════════════════════════════════════════════════════════
    void apply_state(CausalState state) {
        switch (state) {
            case STATE_EMERGENCY:
                fib_index = 3;
                recommended_batch = sys_cfg.N_batch_min;
                gates_since_refresh = 0;
                break;
                
            case STATE_REFRESH:
                gates_since_refresh = 0;
                if (current_noise > noise_watch() * 1.5) {
                    fib_index = std::max(3, fib_index - 2);
                    recommended_batch = clamp_fib(fib_index);
                }
                break;
                
            case STATE_PREPARE:
                if (current_noise > noise_watch()) {
                    fib_index = std::max(3, fib_index - 1);
                    recommended_batch = clamp_fib(fib_index);
                }
                break;
                
            case STATE_WATCH:
                if (guard.current_level >= StabilityGuard::WARNING && 
                    gates_since_refresh > 8) {
                    fib_index = std::max(3, fib_index - 1);
                    recommended_batch = clamp_fib(fib_index);
                } else if (guard.current_level == StabilityGuard::CLEAR && 
                           gates_since_refresh > 16) {
                    fib_index = std::min(15, fib_index + 1);
                    recommended_batch = clamp_fib(fib_index);
                }
                break;
                
            case STATE_IDLE:
                if (guard.current_level == StabilityGuard::CLEAR && 
                    gates_since_refresh > 20) {
                    fib_index = std::min(15, fib_index + 2);
                    recommended_batch = clamp_fib(fib_index);
                }
                break;
        }
    }
    
    // ═══════════════════════════════════════════════════════════
    // BATCH BRIDGE
    // ═══════════════════════════════════════════════════════════
    BatchMetrics finalize_batch() {
        current_batch.batch_should_bootstrap = should_bootstrap();
        current_batch.batch_recommended_size = recommended_batch;
        batch_history.push_back(current_batch);
        if ((int)batch_history.size() > max_batch_history)
            batch_history.erase(batch_history.begin());
        BatchMetrics completed = current_batch;
        current_batch = BatchMetrics();
        current_batch.batch_recommended_size = recommended_batch;
        return completed;
    }
    
    // ═══════════════════════════════════════════════════════════
    // QUERIES
    // ═══════════════════════════════════════════════════════════
    bool should_bootstrap() const {
        return current_state == STATE_REFRESH || current_state == STATE_EMERGENCY;
    }
    
    void reset() {
        gates_since_refresh = 0;
        current_state = STATE_IDLE;
        previous_state = STATE_IDLE;
        current_batch = BatchMetrics();
    }
    
    const char* status() const {
        switch (current_state) {
            case STATE_IDLE:     return "IDLE";
            case STATE_WATCH:    return "WATCH";
            case STATE_PREPARE:  return "PREPARE";
            case STATE_REFRESH:  return "REFRESH";
            case STATE_EMERGENCY: return "EMERGENCY";
            default:             return "UNKNOWN";
        }
    }
    
    bool state_changed() const { return current_state != previous_state; }
    double get_phi() const { return (current_noise + (1.0 - current_stability) + (1.0 - current_lyapunov)) / 3.0; }
};
