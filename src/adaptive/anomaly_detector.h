#pragma once
#include "../core/constants.h"
#include "../utils/safe_math.h"
#include "../utils/logger.h"
#include "system_monitor.h"
#include <deque>
#include <string>

struct AnomalyDetector {
    enum OperationalMode { NOMINAL, ELEVATED, CRITICAL };
    
    OperationalMode current_mode;
    
    double timing_threshold;
    double noise_threshold;
    double stability_threshold;
    
    std::deque<double> anomaly_scores;
    double cumulative_anomaly;
    int consecutive_anomalies;
    
    AnomalyDetector() : current_mode(NOMINAL), timing_threshold(2.0),
                        noise_threshold(1.5), stability_threshold(0.3),
                        cumulative_anomaly(0), consecutive_anomalies(0) {}
    
    void analyze(OperationalMetrics& metrics) {
        double timing_score = metrics.gate_timing.anomaly_score(metrics.gate_time_avg);
        double noise_score = metrics.noise_trend.anomaly_score(metrics.noise_level_avg);
        double stability_drop = 1.0 - metrics.stability_index;
        
        double score = timing_score * PHI / (PHI + 1.0) + 
                      noise_score * 1.0 / (PHI + 1.0);
        
        anomaly_scores.push_back(score);
        if (anomaly_scores.size() > 50) anomaly_scores.pop_front();
        
        cumulative_anomaly = cumulative_anomaly * 0.9 + score * 0.1;
        
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
        if (consecutive_anomalies > 5 || cumulative_anomaly > 0.7) {
            current_mode = CRITICAL;
        } else if (consecutive_anomalies > 2 || cumulative_anomaly > 0.4) {
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
