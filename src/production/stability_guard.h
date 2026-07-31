#pragma once
#include "../utils/logger.h"
#include "../core/constants.h"
#include "../config/system_config.h"
#include <deque>
#include <vector>
#include <string>
#include <sstream>
#include <chrono>
#include <ctime>
#include <algorithm>
#include <cmath>

struct StabilityGuard {

    struct ThresholdConfig {
        double lyapunov_critical     = 0.05;
        double lyapunov_warning      = 0.15;
        double noise_critical        = 0.85;
        double noise_warning         = 0.65;
        double stability_critical    = 0.3;
        double stability_warning     = 0.5;
        int refresh_failure_max      = 3;
        double gate_timeout_seconds  = 10.0;
        int alert_cooldown_seconds   = 5;
        int history_window_size      = 100;
        int prediction_min_samples   = 10;
    };

    ThresholdConfig cfg;

    struct Snapshot {
        double lyapunov, noise, stability, gate_time_sec;
        int refreshes_done, refresh_failures;
        time_t timestamp;
    };

    std::deque<Snapshot> history;
    Snapshot current;

    enum Level { CLEAR = 0, WARNING = 1, CRITICAL = 2 };

    struct Alert {
        Level level;
        std::string component, message;
        double value, threshold;
        time_t when;

        std::string to_json() const {
            std::stringstream ss;
            ss << "{\"level\":\"" << (level==CRITICAL?"critical":"warning")
               << "\",\"component\":\"" << component
               << "\",\"message\":\"" << message
               << "\",\"value\":" << value
               << ",\"threshold\":" << threshold
               << ",\"timestamp\":" << when << "}";
            return ss.str();
        }
    };

    std::vector<Alert> alerts;
    Level current_level = CLEAR;
    std::chrono::steady_clock::time_point last_alert_time;
    int consecutive_failures = 0;

    void init_from_config(const SystemConfig& cfg) {
        this->cfg.lyapunov_critical = cfg.N_lyapunov_critical;
        this->cfg.lyapunov_warning = cfg.N_lyapunov_warning;
        this->cfg.noise_critical = cfg.N_noise_critical;
        this->cfg.noise_warning = cfg.N_noise_warning;
        this->cfg.stability_critical = cfg.N_lyapunov_critical * 6;
        this->cfg.stability_warning = cfg.N_lyapunov_warning * 3.33;
        this->cfg.refresh_failure_max = 3;
        this->cfg.gate_timeout_seconds = 10.0;
        this->cfg.alert_cooldown_seconds = cfg.N_alert_cooldown;
        this->cfg.history_window_size = cfg.N_history_window;
        this->cfg.prediction_min_samples = cfg.N_prediction_min_samples;
    }

    void update(double lyapunov, double noise, double stability,
                double gate_time_sec, int refresh_ok) {
        current.lyapunov = lyapunov; current.noise = noise;
        current.stability = stability; current.gate_time_sec = gate_time_sec;
        current.timestamp = time(0);

        if (refresh_ok) { current.refreshes_done++; consecutive_failures = 0; }
        else { current.refresh_failures++; consecutive_failures++; }

        history.push_back(current);
        if ((int)history.size() > cfg.history_window_size) history.pop_front();
        evaluate_all_checks();
    }

    void evaluate_all_checks() {
        check("lyapunov", current.lyapunov, cfg.lyapunov_warning, cfg.lyapunov_critical, true);
        check("noise", current.noise, cfg.noise_warning, cfg.noise_critical, false);
        check("stability", current.stability, cfg.stability_warning, cfg.stability_critical, true);

        if (current.gate_time_sec > cfg.gate_timeout_seconds) {
            raise(CRITICAL, "gate_timeout",
                  "Gate time " + std::to_string(current.gate_time_sec).substr(0,4) + "s exceeds threshold",
                  current.gate_time_sec, cfg.gate_timeout_seconds);
        }

        if (consecutive_failures >= cfg.refresh_failure_max) {
            raise(CRITICAL, "refresh_failures",
                  std::to_string(consecutive_failures) + " consecutive refresh failures",
                  (double)consecutive_failures, (double)cfg.refresh_failure_max);
        }
    }

    void check(const std::string& name, double value,
               double warn_thresh, double crit_thresh, bool lower_is_worse) {
        bool crit = lower_is_worse ? (value < crit_thresh) : (value > crit_thresh);
        bool warn = lower_is_worse ? (value < warn_thresh) : (value > warn_thresh);

        if (crit) raise(CRITICAL, name, name + " critical: " + std::to_string(value).substr(0,6), value, crit_thresh);
        else if (warn) raise(WARNING, name, name + " degraded: " + std::to_string(value).substr(0,6), value, warn_thresh);
    }

    void raise(Level level, const std::string& component,
               const std::string& msg, double value, double threshold) {
        auto now = std::chrono::steady_clock::now();
        auto since = std::chrono::duration_cast<std::chrono::seconds>(now - last_alert_time).count();
        if (since < cfg.alert_cooldown_seconds && level != CRITICAL) return;

        Alert a{level, component, msg, value, threshold, time(0)};
        alerts.push_back(a);
        if ((int)alerts.size() > 50) alerts.erase(alerts.begin());
        last_alert_time = now;
        current_level = level;

        if (level == CRITICAL) Logger::error("GUARD: " + a.to_json());
        else Logger::warn("GUARD: " + a.to_json());
    }

    double predict_collapse_seconds() {
        int n = history.size();
        if (n < cfg.prediction_min_samples) return -1;
        double sx = 0, sy = 0, sxy = 0, sx2 = 0;
        for (int i = 0; i < n; i++) {
            sx += i; sy += history[i].lyapunov;
            sxy += i * history[i].lyapunov; sx2 += i * i;
        }
        double slope = (n * sxy - sx * sy) / (n * sx2 - sx * sx);
        if (slope >= 0) return -1;
        if (current.lyapunov <= 0) return 0;
        double gates_left = current.lyapunov / (-slope);
        return gates_left * std::max(0.001, current.gate_time_sec);
    }

    std::string metrics_json() const {
        std::stringstream ss;
        ss << "{\"lyapunov\":" << current.lyapunov
           << ",\"noise\":" << current.noise
           << ",\"stability\":" << current.stability
           << ",\"gate_time_sec\":" << current.gate_time_sec
           << ",\"refreshes\":" << current.refreshes_done
           << ",\"failures\":" << current.refresh_failures
           << ",\"alert_level\":" << current_level
           << ",\"alert_count\":" << alerts.size()
           << ",\"uptime_sec\":" << (time(0) - (history.empty() ? time(0) : history.front().timestamp))
           << "}";
        return ss.str();
    }

    std::string metrics_prometheus() const {
        std::stringstream ss;
        ss << "femmgfhe_lyapunov " << current.lyapunov << "\n";
        ss << "femmgfhe_noise " << current.noise << "\n";
        ss << "femmgfhe_stability " << current.stability << "\n";
        ss << "femmgfhe_gate_time_sec " << current.gate_time_sec << "\n";
        ss << "femmgfhe_refreshes " << current.refreshes_done << "\n";
        ss << "femmgfhe_failures " << current.refresh_failures << "\n";
        ss << "femmgfhe_alert_level " << current_level << "\n";
        return ss.str();
    }

    std::string alerts_json() const {
        std::stringstream ss;
        ss << "[";
        for (size_t i = 0; i < alerts.size(); i++) {
            if (i > 0) ss << ",";
            ss << alerts[i].to_json();
        }
        ss << "]";
        return ss.str();
    }

    std::string health_report() const {
        std::stringstream ss;
        ss << "{\"status\":\"" << (current_level == CRITICAL ? "critical" :
                                    current_level == WARNING ? "degraded" : "healthy")
           << "\",\"lyapunov\":" << current.lyapunov
           << ",\"noise\":" << current.noise
           << ",\"stability\":" << current.stability
           << ",\"alerts\":" << alerts.size()
           << ",\"uptime\":" << (time(0) - (history.empty() ? time(0) : history.front().timestamp))
           << "}";
        return ss.str();
    }

    void reset_alerts() {
        alerts.clear();
        current_level = CLEAR;
        consecutive_failures = 0;
    }
};
