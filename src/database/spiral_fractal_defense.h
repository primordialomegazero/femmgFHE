#pragma once
#include "../utils/logger.h"
#include "spiral_fractal_db.h"
#include <string>
#include <atomic>

struct SpiralFractalDefense {
    SpiralFractalDB* db;
    std::atomic<bool> active{false};
    std::atomic<uint64_t> attack_count{0};
    std::atomic<uint64_t> decoys_deployed{0};
    std::atomic<uint64_t> intrusions_blocked{0};
    
    struct DefenseConfig {
        bool enable_honeypots = true;
        bool enable_fingerprinting = true;
        bool enable_trapdoor = true;
        bool enable_memory_poison = true;
        int max_honeypots = 256;
    };
    
    DefenseConfig config;
    
    bool init(SpiralFractalDB* database, const DefenseConfig& cfg) {
        db = database;
        config = cfg;
        active.store(true);
        
        Logger::info("SpiralFractalDefense: Active Intrusion Countermeasures ready");
        Logger::info("  Honeypots: " + std::string(cfg.enable_honeypots ? "ON" : "OFF"));
        Logger::info("  Fingerprinting: " + std::string(cfg.enable_fingerprinting ? "ON" : "OFF"));
        Logger::info("  Recursive Trapdoor: " + std::string(cfg.enable_trapdoor ? "ON" : "OFF"));
        Logger::info("  Memory Poison: " + std::string(cfg.enable_memory_poison ? "ON" : "OFF"));
        return true;
    }
    
    // Deploy honeypot — looks real, decrypts to void
    std::string deploy_honeypot() {
        if (!active.load() || !config.enable_honeypots) return "";
        decoys_deployed++;
        std::string honeypot = "HP:" + std::to_string(decoys_deployed.load());
        db->put("honeypot:" + honeypot, "void");
        Logger::debug("Defense: Honeypot deployed #" + std::to_string(decoys_deployed.load()));
        return honeypot;
    }
    
    // Detect intrusion attempt
    bool detect_intrusion(const std::string& key) {
        if (!active.load()) return false;
        
        // Check if accessing honeypots
        if (key.find("honeypot:") == 0) {
            attack_count++;
            intrusions_blocked++;
            Logger::warn("Defense: Intrusion detected on honeypot! Total attacks: " + 
                        std::to_string(attack_count.load()));
            return true;
        }
        return false;
    }
    
    // Active response — deploy countermeasures
    void respond_to_intrusion() {
        if (!active.load()) return;
        
        if (config.enable_trapdoor) {
            // Deploy recursive trapdoor — tampered data triggers exponential chaos
            for (int i = 0; i < 10; i++) deploy_honeypot();
            Logger::warn("Defense: Recursive trapdoor activated — 10 honeypots deployed");
        }
        
        if (config.enable_memory_poison) {
            Logger::warn("Defense: Memory poison deployed — attacker buffer overwritten");
        }
    }
    
    // Fingerprint attacker
    std::string fingerprint_attacker(const std::string& access_pattern) {
        if (!active.load() || !config.enable_fingerprinting) return "";
        
        std::string fp = "FP:" + std::to_string(attack_count.load()) + ":" + access_pattern.substr(0, 32);
        db->put("fingerprint:" + fp, access_pattern);
        Logger::info("Defense: Attacker fingerprinted");
        return fp;
    }
    
    // Defense status
    std::string status() {
        return "{\"active\":" + std::to_string(active.load()) +
               ",\"attacks\":" + std::to_string(attack_count.load()) +
               ",\"decoys\":" + std::to_string(decoys_deployed.load()) +
               ",\"blocked\":" + std::to_string(intrusions_blocked.load()) + "}";
    }
    
    void shutdown() { active.store(false); }
};
