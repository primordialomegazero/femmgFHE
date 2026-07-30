#pragma once
#include "../core/constants.h"
#include "../utils/safe_math.h"
#include "../utils/logger.h"
#include <chrono>
#include <deque>
#include <algorithm>
#include <numeric>
#include <cmath>

struct TimingWindow {
    std::deque<double> samples;
    size_t max_samples;
    double mean;
    double variance;
    double drift_rate;
    
    TimingWindow(size_t max_s = 20) : max_samples(max_s), mean(0), variance(0), drift_rate(0) {}
    
    void add_sample(double t) {
        samples.push_back(t);
        if (samples.size() > max_samples) samples.pop_front();
        recompute();
    }
    
    void recompute() {
        if (samples.empty()) { mean = 0; variance = 0; return; }
        mean = std::accumulate(samples.begin(), samples.end(), 0.0) / samples.size();
        double var_sum = 0;
        for (double s : samples) var_sum += (s - mean) * (s - mean);
        variance = var_sum / samples.size();
        if (samples.size() > 1) {
            drift_rate = (samples.back() - samples.front()) / samples.size();
        }
    }
    
    double anomaly_score(double current) {
        if (variance < 0.0001) return 0;
        return std::abs(current - mean) / (std::sqrt(variance) + 0.0001);
    }
};

struct OperationalMetrics {
    TimingWindow gate_timing;
    TimingWindow refresh_timing;
    TimingWindow noise_trend;
    
    double gate_time_avg;
    double refresh_time_avg;
    double noise_level_avg;
    double stability_index;
    double performance_index;
    double resource_pressure;
    
    int gates_completed;
    int refreshes_completed;
    int error_count;
    
    OperationalMetrics() : gate_timing(30), refresh_timing(10), noise_trend(20),
                           gate_time_avg(0), refresh_time_avg(0), noise_level_avg(0),
                           stability_index(1.0), performance_index(1.0), resource_pressure(0),
                           gates_completed(0), refreshes_completed(0), error_count(0) {}
    
    void record_gate(double time_taken, double noise_val) {
        gate_timing.add_sample(time_taken);
        noise_trend.add_sample(noise_val);
        gate_time_avg = gate_timing.mean;
        noise_level_avg = noise_trend.mean;
        gates_completed++;
        recompute_indices();
    }
    
    void record_refresh(double time_taken) {
        refresh_timing.add_sample(time_taken);
        refresh_time_avg = refresh_timing.mean;
        refreshes_completed++;
        recompute_indices();
    }
    
    void record_error() {
        error_count++;
        stability_index *= 0.8;
    }
    
    void recompute_indices() {
        stability_index = 1.0 / (1.0 + noise_trend.variance * 10.0 + error_count * 0.1);
        performance_index = 1.0 / (1.0 + gate_time_avg * 0.01);
        resource_pressure = gate_timing.drift_rate > 0.01 ? 0.7 : 
                           gate_timing.drift_rate > 0.001 ? 0.3 : 0.1;
    }
};
