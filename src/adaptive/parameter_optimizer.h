#pragma once
#include "../core/constants.h"
#include "../utils/safe_math.h"
#include "../utils/logger.h"
#include "../config/system_config.h"
#include "system_monitor.h"
#include <string>

struct ParameterOptimizer {
    int fractal_layers, fractal_depth;
    double input_weight;
    int refresh_threshold_gates;
    double phi_state, improvement_rate;
    int optimization_cycle, samples_per_cycle;
    double target_stability, target_performance, target_balance;
    
    // Configurable
    int N_optimizer_layers_start, N_optimizer_depth_start;
    double N_optimizer_weight_start;
    int N_optimizer_refresh_start, N_optimizer_samples_per_cycle;
    double N_optimizer_target_stability, N_optimizer_target_performance;
    int N_optimizer_layers_max, N_optimizer_depth_max;
    double N_optimizer_weight_max, N_optimizer_weight_min;
    int N_optimizer_refresh_min, N_optimizer_refresh_max;
    double N_optimizer_resource_threshold, N_optimizer_stability_threshold;

    ParameterOptimizer() : fractal_layers(5), fractal_depth(3), input_weight(0.75),
                           refresh_threshold_gates(15), phi_state(0.5),
                           improvement_rate(0), optimization_cycle(0),
                           samples_per_cycle(20), target_stability(0.7),
                           target_performance(0.5), target_balance(0.618) {
        N_optimizer_layers_start = 5; N_optimizer_depth_start = 3;
        N_optimizer_weight_start = 0.75; N_optimizer_refresh_start = 15;
        N_optimizer_samples_per_cycle = 20;
        N_optimizer_target_stability = 0.7; N_optimizer_target_performance = 0.5;
        N_optimizer_layers_max = 20; N_optimizer_depth_max = 10;
        N_optimizer_weight_max = 0.9; N_optimizer_weight_min = 0.5;
        N_optimizer_refresh_min = 8; N_optimizer_refresh_max = 40;
        N_optimizer_resource_threshold = 0.6; N_optimizer_stability_threshold = 0.8;
    }
    
    void init_from_config(const SystemConfig& cfg) {
        N_optimizer_layers_start = cfg.N_fractal_layers;
        N_optimizer_depth_start = cfg.N_fractal_depth;
        N_optimizer_weight_start = cfg.N_refresh_weight;
        N_optimizer_refresh_start = cfg.N_alert_cooldown * 3;
        fractal_layers = N_optimizer_layers_start;
        fractal_depth = N_optimizer_depth_start;
        input_weight = N_optimizer_weight_start;
        refresh_threshold_gates = N_optimizer_refresh_start;
        target_balance = cfg.N_refresh_weight;
    }

    void evaluate_and_adapt(OperationalMetrics& metrics) {
        optimization_cycle++;
        if (optimization_cycle % samples_per_cycle != 0) return;

        double current_balance = metrics.stability_index * PHI + metrics.performance_index * PSI;
        phi_state = SafeMath::fmod_safe(phi_state * PHI + current_balance * PSI);
        improvement_rate = (current_balance - target_balance) / (target_balance + 0.001);

        double adjust_threshold_high = PHI / (PHI + 1.0);
        double adjust_threshold_low = PSI / (PSI - 1.0);

        if (std::abs(improvement_rate) < 0.05) return;

        if (improvement_rate > adjust_threshold_high) {
            increase_security(metrics);
        } else if (improvement_rate < -adjust_threshold_low) {
            increase_performance(metrics);
        } else if (metrics.resource_pressure > N_optimizer_resource_threshold) {
            reduce_resource_usage(metrics);
        }

        Logger::info("Optimizer: cycle=" + std::to_string(optimization_cycle) +
                    " balance=" + std::to_string(current_balance).substr(0,4) +
                    " L=" + std::to_string(fractal_layers) +
                    " D=" + std::to_string(fractal_depth) +
                    " R=" + std::to_string(refresh_threshold_gates));
    }

    void increase_security(OperationalMetrics& metrics) {
        if (metrics.stability_index > N_optimizer_stability_threshold && fractal_layers < N_optimizer_layers_max) {
            fractal_layers += fibonacci_increment();
            fractal_depth = std::min(N_optimizer_depth_max, fractal_depth + 1);
            refresh_threshold_gates = std::max(N_optimizer_refresh_min, refresh_threshold_gates - 2);
        }
        input_weight = std::min(N_optimizer_weight_max, input_weight + 0.05);
    }

    void increase_performance(OperationalMetrics& metrics) {
        if (fractal_layers > 3) {
            fractal_layers -= fibonacci_decrement();
            fractal_depth = std::max(1, fractal_depth - 1);
        }
        refresh_threshold_gates = std::min(N_optimizer_refresh_max, refresh_threshold_gates + 3);
        input_weight = std::max(N_optimizer_weight_min, input_weight - 0.05);
    }

    void reduce_resource_usage(OperationalMetrics& metrics) {
        fractal_layers = std::max(3, fractal_layers / 2);
        fractal_depth = std::max(1, fractal_depth - 2);
        refresh_threshold_gates = std::min(N_optimizer_refresh_max, refresh_threshold_gates + 5);
        input_weight = 0.6;
    }

    int fibonacci_increment() {
        static int fib_prev = 1, fib_curr = 1;
        int next = fib_prev + fib_curr;
        fib_prev = fib_curr; fib_curr = next;
        return std::min(8, next);
    }

    int fibonacci_decrement() {
        static int fib_prev = 8, fib_curr = 5;
        int next = std::max(1, fib_prev - fib_curr);
        fib_prev = fib_curr; fib_curr = next;
        return std::max(1, next);
    }
};
