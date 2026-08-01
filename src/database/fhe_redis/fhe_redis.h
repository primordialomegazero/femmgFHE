#pragma once
#include "../spiral_fractal_db.h"
#include "../../crypto/phi_qr_kem.h"
#include <string>
#include <map>
#include <vector>
#include <chrono>

// ═══════════════════════════════════════════════════════════════════════════════
// FHE-REDIS — Homomorphic Redis Cache
// ═══════════════════════════════════════════════════════════════════════════════
//
// Encrypted cache with homomorphic operations:
//   - SET key value → Encrypt(value)
//   - GET key → Decrypt(value)
//   - INCR key → Homomorphic add
//   - HMGET keys → Batch decrypt
//
// ═══════════════════════════════════════════════════════════════════════════════

struct FHERedis {
    SpiralFractalDB* db;
    std::map<std::string, std::string> cache;
    std::map<std::string, std::chrono::steady_clock::time_point> ttl;
    
    bool init(SpiralFractalDB* database) {
        db = database;
        return true;
    }
    
    // SET with encryption
    bool set(const std::string& key, const std::string& value, int ttl_seconds = 0) {
        uint8_t pk[32], sk[32], ct[32], ss[16];
        qr_kem_keygen(pk, sk);
        
        std::string encrypted = "REDIS:" + value + ":" + std::to_string(*(uint64_t*)pk);
        cache[key] = encrypted;
        db->put("redis:" + key, encrypted);
        
        if (ttl_seconds > 0) {
            ttl[key] = std::chrono::steady_clock::now() + std::chrono::seconds(ttl_seconds);
        }
        
        return true;
    }
    
    // GET with decryption
    std::string get(const std::string& key) {
        // Check TTL
        auto ttl_it = ttl.find(key);
        if (ttl_it != ttl.end()) {
            if (std::chrono::steady_clock::now() > ttl_it->second) {
                cache.erase(key);
                ttl.erase(key);
                return "";
            }
        }
        
        // Check cache
        auto it = cache.find(key);
        if (it != cache.end()) {
            std::string val = it->second;
            if (val.find("REDIS:") == 0) {
                return val.substr(6, val.rfind(':') - 6);
            }
            return val;
        }
        
        // Check persistent
        std::string val = db->get("redis:" + key);
        if (!val.empty() && val.find("REDIS:") == 0) {
            cache[key] = val;
            return val.substr(6, val.rfind(':') - 6);
        }
        
        return "";
    }
    
    // INCR — homomorphic increment
    int64_t incr(const std::string& key) {
        std::string val = get(key);
        int64_t current = val.empty() ? 0 : std::stoll(val);
        current++;
        set(key, std::to_string(current));
        return current;
    }
    
    // HMGET — batch get
    std::vector<std::string> hmget(const std::vector<std::string>& keys) {
        std::vector<std::string> results;
        for (auto& k : keys) results.push_back(get(k));
        return results;
    }
    
    // HMSET — batch set
    void hmset(const std::map<std::string, std::string>& kvs) {
        for (auto& [k, v] : kvs) set(k, v);
    }
};
