#pragma once
#include "../utils/logger.h"
#include <string>
#include <map>
#include <functional>
#include <chrono>
#include <thread>

enum class ErrorCode {
    SUCCESS = 0,
    FHE_CONTEXT_FAILED = 100,
    FHE_ENCRYPT_FAILED = 101,
    FHE_DECRYPT_FAILED = 102,
    FHE_NOISE_DEATH = 103,
    GOLDEN_FIBONACCI_FAILED = 200,
    FRACTAL_REFRESH_FAILED = 300,
    SEED_TREE_EXHAUSTED = 400,
    CIRCUIT_EVAL_FAILED = 500,
    TRUTH_TABLE_MISMATCH = 501,
    BATCH_OVERFLOW = 600,
    TIMEOUT = 900,
    UNKNOWN = 999
};

struct ErrorInfo {
    ErrorCode code;
    std::string message;
    int retry_count;
    bool recoverable;
};

struct RetryPolicy {
    int max_retries = 3;
    int base_delay_ms = 100;
    int max_delay_ms = 5000;
    bool exponential_backoff = true;
    
    int get_delay(int attempt) {
        if (!exponential_backoff) return base_delay_ms;
        int delay = base_delay_ms * (1 << attempt);
        return std::min(delay, max_delay_ms);
    }
};

struct ErrorHandler {
    std::map<ErrorCode, RetryPolicy> policies;
    std::map<ErrorCode, int> error_counts;
    
    void set_policy(ErrorCode code, int max_retries, int base_ms, bool exp_backoff = true) {
        policies[code] = {max_retries, base_ms, 5000, exp_backoff};
    }
    
    ErrorInfo handle(ErrorCode code, const std::string& msg) {
        error_counts[code]++;
        auto& policy = policies[code];
        
        ErrorInfo info;
        info.code = code;
        info.message = msg;
        info.retry_count = error_counts[code];
        info.recoverable = (error_counts[code] <= policy.max_retries);
        
        Logger::warn("Error #" + std::to_string((int)code) + " [" + msg + "] " +
                    "attempt=" + std::to_string(error_counts[code]) + "/" + std::to_string(policy.max_retries));
        
        if (info.recoverable) {
            int delay = policy.get_delay(error_counts[code]);
            Logger::info("Retrying in " + std::to_string(delay) + "ms...");
            std::this_thread::sleep_for(std::chrono::milliseconds(delay));
        }
        
        return info;
    }
    
    void reset(ErrorCode code) { error_counts[code] = 0; }
    void reset_all() { error_counts.clear(); }
};
