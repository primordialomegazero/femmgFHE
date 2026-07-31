#pragma once
#include "../utils/logger.h"
#include <string>
#include <chrono>
#include <thread>
#include <algorithm>
#include <cmath>

struct TemporalScheduler {
    int base_batch_size, min_batch, max_batch;
    double acceleration_factor, deceleration_factor;
    double noise_ceiling, noise_floor;
    int stability_window, throttle_delay_us;
    
    enum ScheduleMode { ADAPTIVE, FIXED, AGGRESSIVE, CONSERVATIVE };
    ScheduleMode mode = ADAPTIVE;
    
    int current_batch;
    double noise_history[10];
    int history_index, batches_completed, total_gates;
    
    void init(int base = 50, int min_b = 10, int max_b = 500,
              double accel = 2.0, double decel = 0.5,
              double ceil_n = 0.7, double floor_n = 0.3) {
        base_batch_size = base; min_batch = min_b; max_batch = max_b;
        acceleration_factor = accel; deceleration_factor = decel;
        noise_ceiling = ceil_n; noise_floor = floor_n;
        stability_window = 10; throttle_delay_us = 0;
        current_batch = base; history_index = 0; batches_completed = 0; total_gates = 0;
        for (int i = 0; i < 10; i++) noise_history[i] = 0.5;
    }
    
    int get_window_size(double current_noise) {
        noise_history[history_index] = current_noise;
        history_index = (history_index + 1) % 10;
        
        double avg_noise = 0;
        for (int i = 0; i < 10; i++) avg_noise += noise_history[i];
        avg_noise /= 10.0;
        double noise_trend = current_noise - avg_noise;
        
        switch (mode) {
            case FIXED: return base_batch_size;
            case AGGRESSIVE:
                if (current_noise < noise_floor * 1.5)
                    current_batch = std::min((int)(current_batch * acceleration_factor * 1.5), max_batch);
                else if (current_noise > noise_ceiling * 0.8)
                    current_batch = std::max((int)(current_batch * deceleration_factor * 0.8), min_batch);
                break;
            case CONSERVATIVE:
                if (current_noise > noise_ceiling * 0.7)
                    current_batch = std::max((int)(current_batch * deceleration_factor * 0.6), min_batch);
                else if (current_noise < noise_floor * 1.8 && noise_trend < 0)
                    current_batch = std::min((int)(current_batch * 1.2), max_batch);
                break;
            case ADAPTIVE:
            default:
                if (noise_trend > 0.1 && current_noise > noise_ceiling * 0.8) {
                    double severity = (current_noise - noise_floor) / (noise_ceiling - noise_floor);
                    double factor = 1.0 - severity * (1.0 - deceleration_factor);
                    current_batch = std::max((int)(current_batch * factor), min_batch);
                } else if (noise_trend < -0.05 && current_noise < noise_ceiling * 0.6) {
                    double headroom = (noise_ceiling - current_noise) / (noise_ceiling - noise_floor);
                    double factor = 1.0 + headroom * (acceleration_factor - 1.0);
                    current_batch = std::min((int)(current_batch * factor), max_batch);
                }
                break;
        }
        
        if (throttle_delay_us > 0)
            std::this_thread::sleep_for(std::chrono::microseconds(throttle_delay_us));
        
        batches_completed++;
        return current_batch;
    }
    
    int predict_next_batch() {
        double trend = 0;
        for (int i = 1; i < 10; i++) {
            int idx1 = (history_index - i + 10) % 10;
            int idx2 = (history_index - i - 1 + 10) % 10;
            trend += noise_history[idx1] - noise_history[idx2];
        }
        trend /= 9.0;
        if (trend > 0.05) return std::max(current_batch / 2, min_batch);
        if (trend < -0.05) return std::min(current_batch * 2, max_batch);
        return current_batch;
    }
    
    void report() {
        double avg = 0;
        for (int i = 0; i < 10; i++) avg += noise_history[i];
        avg /= 10.0;
        Logger::info("TemporalScheduler: batch=" + std::to_string(current_batch) +
                    " avg_noise=" + std::to_string(avg).substr(0,4) +
                    " mode=" + mode_string() +
                    " batches=" + std::to_string(batches_completed));
    }
    
    std::string mode_string() {
        switch(mode) { 
            case ADAPTIVE: return "ADAPTIVE";
            case FIXED: return "FIXED";
            case AGGRESSIVE: return "AGGRESSIVE";
            case CONSERVATIVE: return "CONSERVATIVE";
        }
        return "UNKNOWN";
    }
};
