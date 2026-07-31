#pragma once
#include "../utils/logger.h"
#include <fstream>
#include <sstream>
#include <string>
#include <map>
#include <ctime>

struct ChainState {
    int gates_completed = 0;
    int refreshes_done = 0;
    double phi_value = 0;
    double psi_value = 0;
    int fractal_layers = 5;
    int fractal_depth = 3;
    double noise_level = 0;
    std::string mode = "NOMINAL";
    time_t timestamp = 0;
    
    std::string serialize() const {
        std::stringstream ss;
        ss << gates_completed << "\n"
           << refreshes_done << "\n"
           << phi_value << "\n"
           << psi_value << "\n"
           << fractal_layers << "\n"
           << fractal_depth << "\n"
           << noise_level << "\n"
           << mode << "\n"
           << timestamp << "\n";
        return ss.str();
    }
    
    bool deserialize(const std::string& data) {
        std::stringstream ss(data);
        ss >> gates_completed >> refreshes_done >> phi_value >> psi_value
           >> fractal_layers >> fractal_depth >> noise_level >> mode >> timestamp;
        return !ss.fail();
    }
};

struct StatePersistence {
    std::string filepath;
    
    void init(const std::string& path = "femmgFHE_state.sav") {
        filepath = path;
        Logger::info("State persistence: " + filepath);
    }
    
    bool save(const ChainState& state) {
        std::ofstream file(filepath);
        if (!file.is_open()) {
            Logger::error("Failed to open state file for writing: " + filepath);
            return false;
        }
        file << state.serialize();
        file.close();
        Logger::info("State saved: " + std::to_string(state.gates_completed) + " gates");
        return true;
    }
    
    bool load(ChainState& state) {
        std::ifstream file(filepath);
        if (!file.is_open()) {
            Logger::info("No previous state file found");
            return false;
        }
        std::stringstream buffer;
        buffer << file.rdbuf();
        file.close();
        
        if (state.deserialize(buffer.str())) {
            Logger::info("State loaded: " + std::to_string(state.gates_completed) + " gates");
            return true;
        }
        Logger::warn("State file corrupted, starting fresh");
        return false;
    }
    
    bool exists() {
        std::ifstream file(filepath);
        return file.good();
    }
};
