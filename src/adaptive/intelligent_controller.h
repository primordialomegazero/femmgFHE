#pragma once
#include "../utils/logger.h"
#include "../utils/safe_math.h"
#include "../core/constants.h"
#include "../config/system_config.h"
#include "../production/stability_guard.h"
#include "../production/temporal_scheduler.h"
#include "../refresh/emergent_refresh.h"
#include "system_monitor.h"
#include "parameter_optimizer.h"
#include "anomaly_detector.h"
#include <deque>
#include <string>
#include <sstream>
#include <cmath>

struct AutonomousManager {
    StabilityGuard guard;
    TemporalScheduler scheduler;
    ParameterOptimizer optimizer;
    AnomalyDetector detector;
    OperationalMetrics metrics;
    EmergentRefreshController* refresh_ctrl;
    
    // Configurable (from SystemConfig)
    double N_lyapunov_warning, N_lyapunov_critical, N_noise_warning, N_noise_critical;
    double N_stability_warning, N_stability_critical;
    int N_batch_start, N_batch_min, N_batch_max;
    double N_accel, N_decel, N_ceil, N_floor;
    double N_emergency_noise, N_emergency_stability, N_emergency_lyapunov;
    double N_proactive_noise_base, N_proactive_stability_base;
    double N_adaptive_noise_high, N_adaptive_noise_low;
    double N_gate_time_slow;
    int N_safety_gate_max;

    struct {
        double lyapunov, noise, stability, gate_time;
        int gates_since_refresh, total_gates, refreshes, failures;
        bool should_refresh, chain_alive;
        int recommended_batch;
        std::string mode, phase;
        double phi_state;
        double decision_phi;
        int consecutive_reduces;
    } state;

    struct FailureRecord {
        int gate_number; std::string cause;
        double noise, stability, lyapunov;
        std::string action_taken; bool recovered;
    };
    std::deque<FailureRecord> failure_log;

    enum Action {
        DO_NOTHING, REFRESH_NOW, REDUCE_BATCH, INCREASE_BATCH,
        INCREASE_SECURITY, REDUCE_COMPLEXITY, EMERGENCY_RECOVERY
    };

    Action last_action = DO_NOTHING;
    std::string last_reason;

    void init(EmergentRefreshController* rc = nullptr) {
        refresh_ctrl = rc;
        
        // Defaults (overridden by init_from_config)
        N_lyapunov_warning = 0.15; N_lyapunov_critical = 0.05;
        N_noise_warning = 0.45; N_noise_critical = 0.7;
        N_stability_warning = 0.5; N_stability_critical = 0.3;
        N_batch_start = 50; N_batch_min = 5; N_batch_max = 500;
        N_accel = 2.0; N_decel = 0.4; N_ceil = 0.65; N_floor = 0.3;
        N_emergency_noise = 0.85; N_emergency_stability = 0.15; N_emergency_lyapunov = 0.02;
        N_proactive_noise_base = 0.55; N_proactive_stability_base = 0.5;
        N_adaptive_noise_high = 0.55; N_adaptive_noise_low = 0.3;
        N_gate_time_slow = 8.0; N_safety_gate_max = 35;
        
        guard.cfg.lyapunov_warning = N_lyapunov_warning;
        guard.cfg.lyapunov_critical = N_lyapunov_critical;
        guard.cfg.noise_warning = N_noise_warning;
        guard.cfg.noise_critical = N_noise_critical;
        guard.cfg.stability_warning = N_stability_warning;
        guard.cfg.stability_critical = N_stability_critical;

        scheduler.init(N_batch_start, N_batch_min, N_batch_max, N_accel, N_decel, N_ceil, N_floor);
        scheduler.mode = TemporalScheduler::ADAPTIVE;

        state.chain_alive = true;
        state.recommended_batch = N_batch_start;
        state.phase = "INIT";
        state.gates_since_refresh = 0;
        state.phi_state = 0.5;
        state.decision_phi = PHI / (PHI + 1.0);
        state.consecutive_reduces = 0;

        Logger::info("AutonomousManager: IIT+Phi optimization active. System self-manages.");
    }
    
    void init_from_config(const SystemConfig& cfg) {
        N_lyapunov_warning = cfg.N_lyapunov_warning;
        N_lyapunov_critical = cfg.N_lyapunov_critical;
        N_noise_warning = cfg.N_noise_warning;
        N_noise_critical = cfg.N_noise_critical;
        N_stability_warning = cfg.N_lyapunov_warning * 3.33;
        N_stability_critical = cfg.N_lyapunov_critical * 6;
        N_batch_start = cfg.N_batch_size;
        N_batch_min = cfg.N_batch_min;
        N_batch_max = cfg.N_batch_max;
        
        init(nullptr);
    }

    Action sense(double lyapunov, double noise, double stability, double gate_time) {
        state.lyapunov = lyapunov;
        state.noise = noise;
        state.stability = stability;
        state.gate_time = gate_time;
        state.gates_since_refresh++;
        state.total_gates++;

        guard.update(lyapunov, noise, stability, gate_time, 1);
        metrics.record_gate(gate_time, noise);
        if (state.total_gates % 10 == 0) detector.analyze(metrics);

        update_phi_state();
        Action action = evaluate();
        execute(action);
        state.phase = detector.mode_string();
        return action;
    }

    void update_phi_state() {
        double integrated = state.noise * (1.0 - state.stability) +
                           (1.0 - state.lyapunov) * 0.3;
        state.phi_state = SafeMath::fmod_safe(state.phi_state * PHI + integrated * PSI);
        state.decision_phi = state.phi_state * 0.4 + 0.3;
    }

    Action evaluate() {
        if (state.noise > N_emergency_noise || state.stability < N_emergency_stability || state.lyapunov < N_emergency_lyapunov) {
            last_reason = "CRITICAL: noise=" + std::to_string(state.noise).substr(0,4) +
                         " stability=" + std::to_string(state.stability).substr(0,4);
            return EMERGENCY_RECOVERY;
        }

        double phi_weight = state.decision_phi;
        bool noise_trigger = (state.noise > N_proactive_noise_base + phi_weight * 0.3) && (state.gates_since_refresh > 8);
        bool stability_trigger = (state.stability < N_proactive_stability_base - phi_weight * 0.3) && (state.gates_since_refresh > 8);
        bool guard_trigger = (guard.current_level >= StabilityGuard::WARNING) && (state.gates_since_refresh > 10);
        bool consecutive_trigger = (state.consecutive_reduces > 3);

        if (noise_trigger || stability_trigger || guard_trigger || consecutive_trigger) {
            last_reason = "PHI-REFRESH: phi=" + std::to_string(phi_weight).substr(0,4) +
                         " noise=" + std::to_string(state.noise).substr(0,4) +
                         " reduces=" + std::to_string(state.consecutive_reduces);
            return REFRESH_NOW;
        }

        if (state.noise > N_adaptive_noise_high) {
            state.consecutive_reduces++;
            last_reason = "Noise elevated: " + std::to_string(state.noise).substr(0,4);
            return REDUCE_BATCH;
        } else {
            state.consecutive_reduces = std::max(0, state.consecutive_reduces - 1);
        }

        if (state.noise < N_adaptive_noise_low && state.gates_since_refresh > 15) {
            last_reason = "Low noise — increasing batch";
            return INCREASE_BATCH;
        }

        if (detector.is_elevated() && state.gates_since_refresh > 15) {
            last_reason = "Elevated mode — increasing security";
            return INCREASE_SECURITY;
        }
        if (state.gate_time > N_gate_time_slow && optimizer.fractal_layers > 5) {
            last_reason = "Slow gates — reducing complexity";
            return REDUCE_COMPLEXITY;
        }

        if (state.gates_since_refresh > N_safety_gate_max) {
            last_reason = "Gate limit: " + std::to_string(state.gates_since_refresh);
            return REFRESH_NOW;
        }

        return DO_NOTHING;
    }

    void execute(Action action) {
        last_action = action;
        state.should_refresh = false;

        switch (action) {
            case REFRESH_NOW:
                state.should_refresh = true;
                state.gates_since_refresh = 0;
                state.refreshes++;
                state.consecutive_reduces = 0;
                guard.reset_alerts();
                break;

            case EMERGENCY_RECOVERY:
                state.should_refresh = true;
                state.gates_since_refresh = 0;
                state.refreshes++;
                state.consecutive_reduces = 0;
                optimizer.reduce_resource_usage(metrics);
                state.recommended_batch = 5;
                guard.reset_alerts();
                Logger::warn("AUTONOMOUS: EMERGENCY — " + last_reason +
                           " | L=" + std::to_string(optimizer.fractal_layers) +
                           " D=" + std::to_string(optimizer.fractal_depth));
                break;

            case REDUCE_BATCH:
                state.recommended_batch = std::max(5, state.recommended_batch / 2);
                break;

            case INCREASE_BATCH:
                state.recommended_batch = std::min(500, state.recommended_batch * 2);
                state.consecutive_reduces = 0;
                break;

            case INCREASE_SECURITY:
                optimizer.increase_security(metrics);
                break;

            case REDUCE_COMPLEXITY:
                optimizer.increase_performance(metrics);
                break;

            default: break;
        }
    }

    FailureRecord diagnose_failure() {
        FailureRecord fr;
        fr.gate_number = state.total_gates;
        fr.noise = state.noise; fr.stability = state.stability; fr.lyapunov = state.lyapunov;
        fr.recovered = false;
        if (state.noise > 0.9) { fr.cause = "NOISE_DEATH"; fr.action_taken = "Increase layers, lower threshold"; }
        else if (state.stability < 0.15) { fr.cause = "STABILITY_COLLAPSE"; fr.action_taken = "Increase input_weight"; }
        else if (state.lyapunov < 0.01) { fr.cause = "CHAOS_COLLAPSE"; fr.action_taken = "Adjust logistic r"; }
        else if (state.refreshes == 0) { fr.cause = "NO_REFRESH"; fr.action_taken = "Lower proactive thresholds"; }
        else { fr.cause = "NOISE_DEATH"; fr.action_taken = "Increase security parameters"; }
        failure_log.push_back(fr);
        if (failure_log.size() > 20) failure_log.pop_front();
        return fr;
    }

    void self_heal() {
        auto f = diagnose_failure();
        optimizer.fractal_layers = std::min(30, optimizer.fractal_layers + 5);
        optimizer.fractal_depth = std::min(10, optimizer.fractal_depth + 2);
        state.recommended_batch = 10;
        state.chain_alive = true;
        state.gates_since_refresh = 0;
        state.consecutive_reduces = 0;
        guard.reset_alerts();
        Logger::info("AUTONOMOUS: HEALED — L=" + std::to_string(optimizer.fractal_layers) +
                    " D=" + std::to_string(optimizer.fractal_depth) +
                    " batch=" + std::to_string(state.recommended_batch));
    }

    std::string full_report() {
        std::stringstream ss;
        ss << "{\"gates\":" << state.total_gates
           << ",\"refreshes\":" << state.refreshes
           << ",\"noise\":" << state.noise
           << ",\"stability\":" << state.stability
           << ",\"phi\":" << state.phi_state
           << ",\"batch\":" << state.recommended_batch
           << ",\"layers\":" << optimizer.fractal_layers
           << ",\"depth\":" << optimizer.fractal_depth
           << ",\"phase\":\"" << state.phase << "\""
           << ",\"failures\":" << failure_log.size() << "}";
        return ss.str();
    }
};
