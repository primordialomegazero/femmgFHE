#pragma once
#include "../core/constants.h"
#include "../utils/safe_math.h"
#include "../utils/logger.h"
#include "../config/system_config.h"
#include "system_monitor.h"
#include <deque>
#include <string>

struct AnomalyDetector {
    enum OperationalMode { NOMINAL, ELEVATED, CRITICAL };
    OperationalMode current_mode;
    double timing_threshold, noise_threshold, stability_threshold;
    std::deque<double> anomaly_scores;
    double cumulative_anomaly;
    int consecutive_anomalies;
    
    // Configurable
    double N_anomaly_timing_threshold, N_anomaly_noise_threshold;
    double N_anomaly_stability_threshold, N_anomaly_decay_rate;
    double N_anomaly_learn_rate;
    int N_anomaly_critical_consecutive, N_anomaly_elevated_consecutive;
    double N_anomaly_critical_cumulative, N_anomaly_elevated_cumulative;

    AnomalyDetector() : current_mode(NOMINAL), timing_threshold(2.0),
                        noise_threshold(1.5), stability_threshold(0.3),
                        cumulative_anomaly(0), consecutive_anomalies(0) {
        N_anomaly_timing_threshold = 2.0;
        N_anomaly_noise_threshold = 1.5;
        N_anomaly_stability_threshold = 0.3;
        N_anomaly_decay_rate = 0.9;
        N_anomaly_learn_rate = 0.1;
        N_anomaly_critical_consecutive = 5;
        N_anomaly_elevated_consecutive = 2;
        N_anomaly_critical_cumulative = 0.7;
        N_anomaly_elevated_cumulative = 0.4;
    }
    
    void init_from_config(const SystemConfig& cfg) {
        // Auto-scale from config
        N_anomaly_stability_threshold = cfg.N_lyapunov_critical * 6;
        N_anomaly_critical_consecutive = cfg.N_alert_cooldown;
    }

    void analyze(OperationalMetrics& metrics) {
        double timing_score = metrics.gate_timing.anomaly_score(metrics.gate_time_avg);
        double noise_score = metrics.noise_trend.anomaly_score(metrics.noise_level_avg);
        double stability_drop = 1.0 - metrics.stability_index;

        double score = timing_score * PHI / (PHI + 1.0) + noise_score * 1.0 / (PHI + 1.0);
        anomaly_scores.push_back(score);
        if (anomaly_scores.size() > 50) anomaly_scores.pop_front();
        cumulative_anomaly = cumulative_anomaly * N_anomaly_decay_rate + score * N_anomaly_learn_rate;

        if (score > timing_threshold || stability_drop > stability_threshold) {
            consecutive_anomalies++;
        } else {
            consecutive_anomalies = std::max(0, consecutive_anomalies - 1);
        }

        update_mode(score, stability_drop);

        if (current_mode != NOMINAL && consecutive_anomalies == 1) {
            Logger::warn("AnomalyDetector: mode=" + mode_string() +
                        " score=" + std::to_string(score).substr(0,4) +
                        " cum=" + std::to_string(cumulative_anomaly).substr(0,4));
        }
    }

    void update_mode(double score, double stability_drop) {
        if (consecutive_anomalies > N_anomaly_critical_consecutive || cumulative_anomaly > N_anomaly_critical_cumulative) {
            current_mode = CRITICAL;
        } else if (consecutive_anomalies > N_anomaly_elevated_consecutive || cumulative_anomaly > N_anomaly_elevated_cumulative) {
            current_mode = ELEVATED;
        } else {
            current_mode = NOMINAL;
        }
    }

    std::string mode_string() {
        switch (current_mode) {
            case NOMINAL: return "NOMINAL";
            case ELEVATED: return "ELEVATED";
            case CRITICAL: return "CRITICAL";
        }
        return "UNKNOWN";
    }

    bool is_elevated() { return current_mode >= ELEVATED; }
    bool is_critical() { return current_mode == CRITICAL; }
};
