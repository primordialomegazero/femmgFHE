#pragma once
#include "../utils/logger.h"
#include <string>
#include <chrono>
#include <map>

struct HealthStatus {
    bool fhe_ok = false;
    bool seed_tree_ok = false;
    bool fractal_ok = false;
    bool chain_active = false;
    int gates_completed = 0;
    int refreshes_done = 0;
    double uptime_seconds = 0;
    std::string mode = "INIT";
    
    std::string to_json() {
        std::string json = "{";
        json += "\"status\":\"" + std::string(chain_active ? "healthy" : "degraded") + "\",";
        json += "\"fhe\":" + std::string(fhe_ok ? "true" : "false") + ",";
        json += "\"seed_tree\":" + std::string(seed_tree_ok ? "true" : "false") + ",";
        json += "\"fractal\":" + std::string(fractal_ok ? "true" : "false") + ",";
        json += "\"gates\":" + std::to_string(gates_completed) + ",";
        json += "\"refreshes\":" + std::to_string(refreshes_done) + ",";
        json += "\"uptime\":" + std::to_string(uptime_seconds) + ",";
        json += "\"mode\":\"" + mode + "\"";
        json += "}";
        return json;
    }
    
    std::string to_prometheus() {
        std::string metrics;
        metrics += "femmgfhe_fhe_ok " + std::string(fhe_ok ? "1" : "0") + "\n";
        metrics += "femmgfhe_seed_tree_ok " + std::string(seed_tree_ok ? "1" : "0") + "\n";
        metrics += "femmgfhe_fractal_ok " + std::string(fractal_ok ? "1" : "0") + "\n";
        metrics += "femmgfhe_chain_active " + std::string(chain_active ? "1" : "0") + "\n";
        metrics += "femmgfhe_gates_completed " + std::to_string(gates_completed) + "\n";
        metrics += "femmgfhe_refreshes_done " + std::to_string(refreshes_done) + "\n";
        metrics += "femmgfhe_uptime_seconds " + std::to_string(uptime_seconds) + "\n";
        return metrics;
    }
};

struct HealthCheck {
    HealthStatus status;
    std::chrono::steady_clock::time_point start_time;
    
    void init() {
        start_time = std::chrono::steady_clock::now();
        status.mode = "STARTING";
    }
    
    void update() {
        auto now = std::chrono::steady_clock::now();
        status.uptime_seconds = std::chrono::duration<double>(now - start_time).count();
    }
    
    std::string health() {
        update();
        return status.to_json();
    }
    
    std::string ready() {
        update();
        if (status.fhe_ok && status.seed_tree_ok && status.fractal_ok) {
            status.mode = "READY";
            return "OK";
        }
        status.mode = "NOT_READY";
        return "NOT READY";
    }
    
    std::string metrics() {
        update();
        return status.to_prometheus();
    }
};
