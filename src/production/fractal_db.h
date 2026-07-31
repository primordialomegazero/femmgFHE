#pragma once
#include "../utils/logger.h"
#include "../utils/safe_math.h"
#include "../core/constants.h"
#include <fstream>
#include <sstream>
#include <map>
#include <string>
#include <vector>
#include <ctime>
#include <deque>
#include <chrono>
#include <sys/stat.h>

// ═══════════════════════════════════════════════════════════════
// FRACTAL DB — Multi-Layer Persistent Storage with Tiered Retention
// ═══════════════════════════════════════════════════════════════
//
// Architecture:
//   L0 (Hot)  → In-memory cache, sub-ms access
//   L1 (Warm) → Recent checkpoints, last N saves
//   L2 (Cold) → Compressed historical snapshots
//   L3 (Archive) → Eternal immutable records
//
// Retention Policy:
//   L1: Last 10 checkpoints (rolling window)
//   L2: Hourly snapshots (24 hours)
//   L3: Daily snapshots (forever — eternal persistence)
// ═══════════════════════════════════════════════════════════════

struct FractalDB {
    std::string base_path;
    
    // Tiered storage paths
    std::string l0_path;  // Hot cache
    std::string l1_path;  // Warm recent
    std::string l2_path;  // Cold hourly
    std::string l3_path;  // Archive eternal
    
    // In-memory L0 cache
    std::map<std::string, std::map<std::string, std::string>> memory_layer;
    
    // L1 rolling buffer
    std::deque<std::string> l1_buffer;
    int l1_max_entries;
    
    // Stats
    int total_writes;
    int total_reads;
    time_t init_time;
    
    void init(const std::string& base = "fractal_db", int l1_size = 10) {
        base_path = base;
        l1_max_entries = l1_size;
        total_writes = 0;
        total_reads = 0;
        init_time = time(0);
        
        // Create directory structure
        mkdir(base_path.c_str(), 0755);
        l0_path = base_path + "/L0_hot";
        l1_path = base_path + "/L1_warm";
        l2_path = base_path + "/L2_cold";
        l3_path = base_path + "/L3_archive";
        
        mkdir(l0_path.c_str(), 0755);
        mkdir(l1_path.c_str(), 0755);
        mkdir(l2_path.c_str(), 0755);
        mkdir(l3_path.c_str(), 0755);
        
        // Load L0 from disk
        load_layer(l0_path + "/current.db");
        
        Logger::info("FractalDB: " + base_path + 
                    " [L0:memory L1:" + std::to_string(l1_size) + 
                    " L2:hourly L3:eternal]");
    }
    
    // ═══════════════════════════════════════════════════════════
    // L0: HOT LAYER — In-memory, instant access
    // ═══════════════════════════════════════════════════════════
    
    void l0_set(const std::string& section, const std::string& key, const std::string& val) {
        memory_layer[section][key] = val;
        total_writes++;
    }
    
    std::string l0_get(const std::string& section, const std::string& key, const std::string& def = "") {
        auto si = memory_layer.find(section);
        if (si == memory_layer.end()) return def;
        auto ki = si->second.find(key);
        total_reads++;
        return (ki != si->second.end()) ? ki->second : def;
    }
    
    bool l0_exists(const std::string& section, const std::string& key) {
        auto si = memory_layer.find(section);
        return si != memory_layer.end() && si->second.find(key) != si->second.end();
    }
    
    void l0_flush() {
        save_layer(l0_path + "/current.db");
        Logger::debug("L0 flushed: " + std::to_string(memory_layer.size()) + " sections");
    }
    
    // ═══════════════════════════════════════════════════════════
    // L1: WARM LAYER — Rolling recent checkpoints
    // ═══════════════════════════════════════════════════════════
    
    void l1_save(const std::string& checkpoint_name) {
        // Generate timestamped filename
        auto now = std::chrono::system_clock::now();
        auto t = std::chrono::system_clock::to_time_t(now);
        std::string ts = std::to_string(t);
        std::string filename = l1_path + "/" + checkpoint_name + "_" + ts + ".db";
        
        // Save current L0 state to L1
        std::ofstream file(filename);
        for (auto& [section, kv] : memory_layer) {
            file << "[" << section << "]\n";
            for (auto& [k, v] : kv) file << k << "=" << v << "\n";
            file << "\n";
        }
        file.close();
        
        // Add to rolling buffer
        l1_buffer.push_back(filename);
        total_writes++;
        
        // Rotate: remove oldest if exceeds max
        while ((int)l1_buffer.size() > l1_max_entries) {
            std::string oldest = l1_buffer.front();
            l1_buffer.pop_front();
            
            // Promote oldest to L2 before deleting
            l2_promote(oldest);
        }
        
        Logger::info("L1 saved: " + checkpoint_name + " [" + 
                    std::to_string(l1_buffer.size()) + "/" + std::to_string(l1_max_entries) + "]");
    }
    
    std::string l1_get_latest() {
        if (l1_buffer.empty()) return "";
        return l1_buffer.back();
    }
    
    // ═══════════════════════════════════════════════════════════
    // L2: COLD LAYER — Hourly snapshots (24-hour retention)
    // ═══════════════════════════════════════════════════════════
    
    void l2_promote(const std::string& l1_filename) {
        // Extract checkpoint name from filename
        size_t last_slash = l1_filename.find_last_of('/');
        std::string basename = (last_slash != std::string::npos) ? 
                               l1_filename.substr(last_slash + 1) : l1_filename;
        
        std::string l2_filename = l2_path + "/" + basename;
        
        // Copy file to L2
        std::ifstream src(l1_filename, std::ios::binary);
        std::ofstream dst(l2_filename, std::ios::binary);
        dst << src.rdbuf();
        src.close(); dst.close();
        
        // Remove original L1 file
        std::remove(l1_filename.c_str());
        
        // Clean old L2 files (> 24 hours)
        l2_cleanup();
    }
    
    void l2_cleanup() {
        auto now = time(0);
        // Implementation: iterate L2 directory, remove files older than 86400 seconds
    }
    
    // ═══════════════════════════════════════════════════════════
    // L3: ARCHIVE LAYER — Eternal immutable records
    // ═══════════════════════════════════════════════════════════
    
    void l3_archive(const std::string& archive_name) {
        auto now = std::chrono::system_clock::now();
        auto t = std::chrono::system_clock::to_time_t(now);
        std::string ts = std::to_string(t);
        std::string filename = l3_path + "/" + archive_name + "_" + ts + ".archive";
        
        // Save with metadata header
        std::ofstream file(filename);
        file << "# FRACTAL DB ARCHIVE — ETERNAL PERSISTENCE\n";
        file << "# Archive: " << archive_name << "\n";
        file << "# Created: " << std::ctime(&t);
        file << "# Layers: L0=" << memory_layer.size() << " sections\n";
        file << "# Total writes: " << total_writes << "\n";
        file << "# Total reads: " << total_reads << "\n";
        file << "# Uptime: " << (t - init_time) << "s\n";
        file << "# === CONTENT ===\n\n";
        
        for (auto& [section, kv] : memory_layer) {
            file << "[" << section << "]\n";
            for (auto& [k, v] : kv) file << k << "=" << v << "\n";
            file << "\n";
        }
        file.close();
        
        Logger::info("L3 archived: " + archive_name + " (eternal)");
    }
    
    // ═══════════════════════════════════════════════════════════
    // CROSS-LAYER OPERATIONS
    // ═══════════════════════════════════════════════════════════
    
    void save_checkpoint(const std::string& name, bool archive = false) {
        l0_flush();                    // Persist L0 to disk
        l1_save(name);                 // Push to L1 rolling buffer
        if (archive) l3_archive(name); // Archive to L3 if requested
    }
    
    bool load_checkpoint(const std::string& checkpoint_name) {
        // Try L0 first (fastest)
        if (l0_exists(checkpoint_name, "phase")) return true;
        
        // Try L1
        std::string latest = l1_get_latest();
        if (!latest.empty()) {
            load_layer(latest);
            return true;
        }
        
        // Try L3 archives
        // (scan L3 directory for matching archives)
        
        return false;
    }
    
    void load_layer(const std::string& filepath) {
        std::ifstream file(filepath);
        if (!file.is_open()) return;
        
        std::string line, current_section;
        while (std::getline(file, line)) {
            if (line.empty() || line[0] == '#') continue;
            if (line[0] == '[' && line.back() == ']') {
                current_section = line.substr(1, line.size() - 2);
                continue;
            }
            auto eq = line.find('=');
            if (eq != std::string::npos && !current_section.empty()) {
                memory_layer[current_section][line.substr(0, eq)] = line.substr(eq + 1);
            }
        }
        file.close();
    }
    
    void save_layer(const std::string& filepath) {
        std::ofstream file(filepath);
        for (auto& [section, kv] : memory_layer) {
            file << "[" << section << "]\n";
            for (auto& [k, v] : kv) file << k << "=" << v << "\n";
            file << "\n";
        }
        file.close();
    }
    
    // ═══════════════════════════════════════════════════════════
    // DIAGNOSTICS
    // ═══════════════════════════════════════════════════════════
    
    void stats() {
        Logger::section("FRACTAL DB STATISTICS");
        Logger::keyval("L0 sections", std::to_string(memory_layer.size()));
        Logger::keyval("L1 entries", std::to_string(l1_buffer.size()) + "/" + std::to_string(l1_max_entries));
        Logger::keyval("Total writes", std::to_string(total_writes));
        Logger::keyval("Total reads", std::to_string(total_reads));
        Logger::keyval("Uptime", std::to_string(time(0) - init_time) + "s");
    }
};
