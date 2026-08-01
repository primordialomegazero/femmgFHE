#pragma once
#include "../utils/logger.h"
#include "../utils/safe_math.h"
#include "../core/constants.h"
#include "../config/system_config.h"
#include "../production/stability_guard.h"
#include "../production/temporal_scheduler.h"
#include "../production/fractal_db.h"
#include "../refresh/emergent_refresh.h"
#include "system_monitor.h"
#include "parameter_optimizer.h"
#include "anomaly_detector.h"
#include <deque>
#include <string>
#include <sstream>
#include <cmath>
#include <ctime>

struct AutonomousController {
    StabilityGuard guard;
    TemporalScheduler scheduler;
    ParameterOptimizer optimizer;
    AnomalyDetector detector;
    OperationalMetrics metrics;
    EmergentRefreshController* refresh_ctrl;
    FractalDB* db;

    // Feature switches
    struct Features {
        bool enable_persistence = true;
        bool enable_learning = true;
        bool enable_prediction = true;
        bool enable_auto_heal = true;
        bool enable_anomaly_detection = true;
        bool enable_phi_state = true;
    } features;

    double N_emergency_noise, N_emergency_stability, N_emergency_lyapunov;
    double N_proactive_noise_base, N_proactive_stability_base;
    double N_adaptive_noise_high, N_adaptive_noise_low;
    int N_safety_gate_max;

    struct {
        double lyapunov, noise, stability, gate_time;
        int gates_since_refresh, total_gates, refreshes, failures;
        bool should_refresh, chain_alive;
        int recommended_batch;
        std::string mode, phase;
        double phi_state, decision_phi;
        int consecutive_reduces;
        time_t start_time;
    } state;

    struct DecisionRecord {
        time_t timestamp; int gate_number;
        double noise, stability, lyapunov; int action_taken;
        std::string action_name; bool was_successful;
    };
    std::deque<DecisionRecord> decision_history;

    struct FailureRecord {
        int gate_number; std::string cause;
        double noise, stability, lyapunov;
        std::string action_taken; bool recovered; time_t timestamp;
    };
    std::deque<FailureRecord> failure_log;

    enum Action {
        DO_NOTHING, REFRESH_NOW, REDUCE_BATCH, INCREASE_BATCH,
        INCREASE_SECURITY, REDUCE_COMPLEXITY, EMERGENCY_RECOVERY,
        SAVE_CHECKPOINT, DEFEND_AGGRESSIVE, PREDICTIVE_ADJUST
    };
    Action last_action;
    std::string last_reason;

    void init(EmergentRefreshController* rc, FractalDB* database, const SystemConfig& cfg) {
        refresh_ctrl = rc;
        db = database;
        
        N_emergency_noise = cfg.N_noise_critical;
        N_emergency_stability = cfg.N_lyapunov_critical * 6;
        N_emergency_lyapunov = cfg.N_lyapunov_critical;
        N_proactive_noise_base = cfg.N_noise_warning;
        N_proactive_stability_base = cfg.N_lyapunov_warning * 3.33;
        N_adaptive_noise_high = cfg.N_noise_warning;
        N_adaptive_noise_low = cfg.N_noise_warning * 0.5;
        N_safety_gate_max = cfg.N_alert_cooldown * 7;

        guard.init_from_config(cfg);
        scheduler.init_from_config(cfg);
        optimizer.init_from_config(cfg);
        detector.init_from_config(cfg);

        state.chain_alive = true;
        state.recommended_batch = cfg.N_batch_size;
        state.start_time = time(0);
        state.phi_state = 0.5;
        state.decision_phi = PHI / (PHI + 1.0);
        state.consecutive_reduces = 0;

        if (features.enable_persistence && db && db->l0_exists("controller", "total_gates")) {
            state.total_gates = std::stoi(db->l0_get("controller", "total_gates", "0"));
            state.refreshes = std::stoi(db->l0_get("controller", "refreshes", "0"));
        }

        last_action = DO_NOTHING;
    }

    Action sense(double lyapunov, double noise, double stability, double gate_time) {
        state.lyapunov = lyapunov; state.noise = noise;
        state.stability = stability; state.gate_time = gate_time;
        state.gates_since_refresh++; state.total_gates++;

        guard.update(lyapunov, noise, stability, gate_time, 1);
        metrics.record_gate(gate_time, noise);

        if (features.enable_anomaly_detection && state.total_gates % 10 == 0) {
            detector.analyze(metrics);
        }
        if (features.enable_phi_state) update_phi_state();

        Action action = evaluate();
        execute(action);
        record_decision(action, lyapunov, noise, stability);

        if (features.enable_persistence && state.total_gates % 50 == 0) save_state();
        if (features.enable_learning && state.total_gates % 100 == 0) learn_from_history();

        state.phase = detector.mode_string();
        return action;
    }

    void update_phi_state() {
        double integrated = state.noise * (1.0 - state.stability) + (1.0 - state.lyapunov) * 0.3;
        state.phi_state = SafeMath::fmod_safe(state.phi_state * PHI + integrated * PSI);
        state.decision_phi = state.phi_state * 0.4 + 0.3;
    }

    Action evaluate() {
        if (state.noise > N_emergency_noise || state.stability < N_emergency_stability || state.lyapunov < N_emergency_lyapunov) {
            return EMERGENCY_RECOVERY;
        }
        if (features.enable_prediction && detector.is_elevated() && state.gates_since_refresh > 10) {
            double eta = guard.predict_collapse_seconds();
            if (eta > 0 && eta < 300) return DEFEND_AGGRESSIVE;
        }
        double pw = state.decision_phi;
        if ((state.noise > N_proactive_noise_base + pw * 0.3 && state.gates_since_refresh > 8) ||
            (state.stability < N_proactive_stability_base - pw * 0.3 && state.gates_since_refresh > 8) ||
            (guard.current_level >= StabilityGuard::WARNING) ||
            (state.consecutive_reduces > 3)) return REFRESH_NOW;
        if (state.noise > N_adaptive_noise_high) { state.consecutive_reduces++; return REDUCE_BATCH; }
        state.consecutive_reduces = std::max(0, state.consecutive_reduces - 1);
        if (state.noise < N_adaptive_noise_low && state.gates_since_refresh > 15) return INCREASE_BATCH;
        if (state.gates_since_refresh > N_safety_gate_max) return REFRESH_NOW;
        return DO_NOTHING;
    }

    void execute(Action action) {
        last_action = action; state.should_refresh = false;
        switch (action) {
            case REFRESH_NOW: state.should_refresh = true; state.gates_since_refresh = 0; state.refreshes++; state.consecutive_reduces = 0; guard.reset_alerts(); break;
            case EMERGENCY_RECOVERY: state.should_refresh = true; state.gates_since_refresh = 0; state.refreshes++; state.consecutive_reduces = 0; optimizer.reduce_resource_usage(metrics); state.recommended_batch = 5; guard.reset_alerts(); save_checkpoint("emergency"); break;
            case DEFEND_AGGRESSIVE: state.should_refresh = true; state.gates_since_refresh = 0; state.refreshes++; optimizer.increase_security(metrics); state.recommended_batch = std::max(5, state.recommended_batch/2); guard.reset_alerts(); save_checkpoint("defend"); break;
            case REDUCE_BATCH: state.recommended_batch = std::max(5, state.recommended_batch/2); break;
            case INCREASE_BATCH: state.recommended_batch = std::min(500, state.recommended_batch*2); state.consecutive_reduces = 0; break;
            case INCREASE_SECURITY: optimizer.increase_security(metrics); break;
            case REDUCE_COMPLEXITY: optimizer.increase_performance(metrics); break;
            default: break;
        }
    }

    void save_state() {
        if (!features.enable_persistence || !db) return;
        db->l0_set("controller", "total_gates", std::to_string(state.total_gates));
        db->l0_set("controller", "refreshes", std::to_string(state.refreshes));
        db->l0_set("controller", "noise", std::to_string(state.noise).substr(0,8));
        db->l0_set("controller", "phi_state", std::to_string(state.phi_state).substr(0,8));
        db->l0_set("controller", "phase", state.phase);
        db->l0_flush();
    }

    void save_checkpoint(const std::string& reason) {
        if (!features.enable_persistence || !db) return;
        save_state(); db->save_checkpoint(reason, true);
    }

    void record_decision(Action action, double lyapunov, double noise, double stability) {
        DecisionRecord dr; dr.timestamp = time(0); dr.gate_number = state.total_gates;
        dr.noise = noise; dr.stability = stability; dr.lyapunov = lyapunov;
        dr.action_taken = (int)action; dr.action_name = action_to_string(action); dr.was_successful = true;
        decision_history.push_back(dr);
        if (decision_history.size() > 100) decision_history.pop_front();
    }

    void learn_from_history() {
        if (!features.enable_learning || decision_history.size() < 10) return;
        int successful = 0;
        for (auto& d : decision_history) if (d.was_successful) successful++;
        double rate = (double)successful / decision_history.size();
        if (rate < 0.5) { N_proactive_noise_base = std::max(0.3, N_proactive_noise_base - 0.05); N_safety_gate_max = std::max(10, N_safety_gate_max - 5); }
        else if (rate > 0.9) { N_proactive_noise_base = std::min(0.7, N_proactive_noise_base + 0.02); N_safety_gate_max = std::min(100, N_safety_gate_max + 5); }
    }

    FailureRecord diagnose_failure() {
        FailureRecord fr; fr.gate_number = state.total_gates; fr.noise = state.noise;
        fr.stability = state.stability; fr.lyapunov = state.lyapunov; fr.timestamp = time(0); fr.recovered = false;
        if (state.noise > 0.9) fr.cause = "NOISE_DEATH";
        else if (state.stability < 0.15) fr.cause = "STABILITY_COLLAPSE";
        else if (state.lyapunov < 0.01) fr.cause = "CHAOS_COLLAPSE";
        else fr.cause = "UNKNOWN";
        failure_log.push_back(fr);
        if (failure_log.size() > 50) failure_log.pop_front();
        return fr;
    }

    void self_heal() {
        if (!features.enable_auto_heal) return;
        auto f = diagnose_failure();
        optimizer.fractal_layers = std::min(30, optimizer.fractal_layers + 5);
        optimizer.fractal_depth = std::min(10, optimizer.fractal_depth + 2);
        state.recommended_batch = 10; state.chain_alive = true;
        state.gates_since_refresh = 0; state.consecutive_reduces = 0;
        state.failures++; guard.reset_alerts();
        save_checkpoint("heal_" + f.cause);
    }

    std::string action_to_string(Action a) {
        switch (a) {
            case DO_NOTHING: return "NOTHING"; case REFRESH_NOW: return "REFRESH";
            case REDUCE_BATCH: return "REDUCE"; case INCREASE_BATCH: return "INCREASE";
            case EMERGENCY_RECOVERY: return "EMERGENCY"; case DEFEND_AGGRESSIVE: return "DEFEND";
            default: return "OTHER";
        }
    }

    std::string full_report() {
        std::stringstream ss;
        ss << "{\"gates\":" << state.total_gates << ",\"refreshes\":" << state.refreshes
           << ",\"noise\":" << state.noise << ",\"stability\":" << state.stability
           << ",\"batch\":" << state.recommended_batch << ",\"layers\":" << optimizer.fractal_layers
           << ",\"phase\":\"" << state.phase << "\"}";
        return ss.str();
    }
};
