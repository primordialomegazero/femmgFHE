#pragma once
#include "../utils/logger.h"
#include <fstream>
#include <sstream>
#include <map>
#include <string>
#include <vector>
#include <ctime>

// ═══════════════════════════════════════════════════════════════
// EMBEDDED DB — Zero-dependency persistent key-value store
// ═══════════════════════════════════════════════════════════════
struct EmbeddedDB {
    std::string filepath;
    std::map<std::string, std::map<std::string, std::string>> sections;
    bool dirty = false;
    
    void init(const std::string& path = "femmgFHE_state.db") {
        filepath = path;
        load_all();
        Logger::info("EmbeddedDB: " + filepath + " (" + std::to_string(sections.size()) + " sections)");
    }
    
    void load_all() {
        sections.clear();
        std::ifstream file(filepath);
        if (!file.is_open()) return;
        
        std::string line, current_section;
        while (std::getline(file, line)) {
            if (line.empty()) continue;
            if (line[0] == '[' && line.back() == ']') {
                current_section = line.substr(1, line.size() - 2);
                continue;
            }
            auto eq = line.find('=');
            if (eq != std::string::npos && !current_section.empty()) {
                std::string key = line.substr(0, eq);
                std::string val = line.substr(eq + 1);
                sections[current_section][key] = val;
            }
        }
        file.close();
    }
    
    void save_all() {
        std::ofstream file(filepath);
        for (auto& [section, kv] : sections) {
            file << "[" << section << "]\n";
            for (auto& [k, v] : kv) {
                file << k << "=" << v << "\n";
            }
            file << "\n";
        }
        file.close();
        dirty = false;
    }
    
    void set(const std::string& section, const std::string& key, const std::string& val) {
        sections[section][key] = val;
        dirty = true;
    }
    
    std::string get(const std::string& section, const std::string& key, const std::string& def = "") {
        auto si = sections.find(section);
        if (si == sections.end()) return def;
        auto ki = si->second.find(key);
        return (ki != si->second.end()) ? ki->second : def;
    }
    
    bool exists(const std::string& section, const std::string& key) {
        auto si = sections.find(section);
        if (si == sections.end()) return false;
        return si->second.find(key) != si->second.end();
    }
    
    void flush() { if (dirty) save_all(); }
    
    std::vector<std::string> list_sections() {
        std::vector<std::string> names;
        for (auto& [name, _] : sections) names.push_back(name);
        return names;
    }
};

// ═══════════════════════════════════════════════════════════════
// CHECKPOINT MANAGER — Auto-save/resume with phase tracking
// ═══════════════════════════════════════════════════════════════
struct CheckpointManager {
    EmbeddedDB db;
    
    enum Phase {
        PHASE_INIT = 0,
        PHASE_TRUTH_TABLE_DONE = 1,
        PHASE_STATS_DONE = 2,
        PHASE_CHAIN_RUNNING = 3,
        PHASE_COMPLETE = 4
    };
    
    struct CheckpointData {
        Phase phase = PHASE_INIT;
        int stats_completed = 0;
        int gates_completed = 0;
        int refreshes_done = 0;
        double phi = 0, psi = 0;
        int fractal_layers = 5, fractal_depth = 3;
        double noise_level = 0;
        time_t timestamp = 0;
    };
    
    void init(const std::string& path = "femmgFHE_checkpoint.db") {
        db.init(path);
    }
    
    void save_checkpoint(const std::string& name, const CheckpointData& data) {
        std::string s = name;
        db.set(s, "phase", std::to_string((int)data.phase));
        db.set(s, "stats_completed", std::to_string(data.stats_completed));
        db.set(s, "gates_completed", std::to_string(data.gates_completed));
        db.set(s, "refreshes_done", std::to_string(data.refreshes_done));
        db.set(s, "phi", std::to_string(data.phi));
        db.set(s, "psi", std::to_string(data.psi));
        db.set(s, "fractal_layers", std::to_string(data.fractal_layers));
        db.set(s, "fractal_depth", std::to_string(data.fractal_depth));
        db.set(s, "noise_level", std::to_string(data.noise_level));
        db.set(s, "timestamp", std::to_string((long)time(0)));
        db.flush();
    }
    
    CheckpointData load_checkpoint(const std::string& name) {
        CheckpointData data;
        std::string s = name;
        if (!db.exists(s, "phase")) return data;
        
        data.phase = (Phase)std::stoi(db.get(s, "phase", "0"));
        data.stats_completed = std::stoi(db.get(s, "stats_completed", "0"));
        data.gates_completed = std::stoi(db.get(s, "gates_completed", "0"));
        data.refreshes_done = std::stoi(db.get(s, "refreshes_done", "0"));
        data.phi = std::stod(db.get(s, "phi", "0"));
        data.psi = std::stod(db.get(s, "psi", "0"));
        data.fractal_layers = std::stoi(db.get(s, "fractal_layers", "5"));
        data.fractal_depth = std::stoi(db.get(s, "fractal_depth", "3"));
        data.noise_level = std::stod(db.get(s, "noise_level", "0"));
        data.timestamp = std::stol(db.get(s, "timestamp", "0"));
        
        return data;
    }
    
    bool has_checkpoint(const std::string& name) {
        return db.exists(name, "phase");
    }
    
    std::string phase_string(Phase p) {
        switch(p) {
            case PHASE_INIT: return "INIT";
            case PHASE_TRUTH_TABLE_DONE: return "TRUTH_TABLE_DONE";
            case PHASE_STATS_DONE: return "STATS_DONE";
            case PHASE_CHAIN_RUNNING: return "CHAIN_RUNNING";
            case PHASE_COMPLETE: return "COMPLETE";
        }
        return "UNKNOWN";
    }
};
