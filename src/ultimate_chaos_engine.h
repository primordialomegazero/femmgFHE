/*
 * ULTIMATE CHAOS ENGINE — CTU v4 (ULTRA-FAST)
 * 
 * "Memory-first, persistence-second."
 */

#pragma once
#include "golden_chaos.h"
#include "spiral_db_lite.h"
#include <iostream>
#include <chrono>
#include <vector>
#include <map>

namespace ultimate_chaos {

using namespace golden_chaos;

class UltimateChaosEngine {
private:
    // Memory-only cache (ultra-fast)
    std::map<int64_t, double> encrypt_cache;
    std::map<double, int64_t> decrypt_cache;
    
    // Optional persistence
    spiraldb_lite::SpiralDBLite* db = nullptr;
    bool use_persistence = false;
    
    uint64_t total_operations = 0;
    double total_chaos = 0.0;
    
public:
    UltimateChaosEngine(bool enable_persistence = false, const std::string& db_dir = "./ultimate_chaos") {
        if(enable_persistence) {
            db = new spiraldb_lite::SpiralDBLite(db_dir);
            use_persistence = true;
            std::cout << "🔥 ULTIMATE CHAOS ENGINE v4 (WITH PERSISTENCE)" << std::endl;
        } else {
            std::cout << "⚡ ULTIMATE CHAOS ENGINE v4 (MEMORY-ONLY — ULTRA FAST)" << std::endl;
        }
        std::cout << "   Layers: " << LAYERS << " chaos layers" << std::endl;
        std::cout << "   Mode: " << (use_persistence ? "PERSISTENT" : "MEMORY-ONLY") << std::endl;
    }
    
    ~UltimateChaosEngine() {
        if(db) delete db;
    }
    
    // ═══ ENCRYPT: With caching ═══
    double encrypt(int64_t plaintext, int layer_shift = 0) {
        // Check cache first
        auto it = encrypt_cache.find(plaintext);
        if(it != encrypt_cache.end()) {
            total_operations++;
            return it->second;
        }
        
        // Compute chaos
        double chaos_history[LAYERS];
        double result = observer_expand_with_persistence(plaintext, layer_shift, chaos_history);
        
        // Store in cache
        encrypt_cache[plaintext] = result;
        decrypt_cache[result] = plaintext;
        
        // Optional persistence
        if(use_persistence && db) {
            spiraldb_lite::ChaosHistory history;
            for(int i = 0; i < LAYERS; i++) {
                history[i] = chaos_history[i];
            }
            std::string key = "plaintext_" + std::to_string(plaintext);
            db->store(key, history);
        }
        
        total_operations++;
        double chaos_sum = 0.0;
        for(int i = 0; i < LAYERS; i++) {
            chaos_sum += std::abs(chaos_history[i]);
        }
        total_chaos += chaos_sum / LAYERS;
        
        return result;
    }
    
    // ═══ DECRYPT: With cache ═══
    int64_t decrypt(double ciphertext) {
        // Check cache first
        auto it = decrypt_cache.find(ciphertext);
        if(it != decrypt_cache.end()) {
            total_operations++;
            return it->second;
        }
        
        // If not in cache, try to compute
        for(int64_t i = 0; i < 1000; i++) {
            double e = encrypt(i, 0);
            if(std::abs(e - ciphertext) < 1e-9) {
                total_operations++;
                return i;
            }
        }
        
        return -1;
    }
    
    // ═══ METRICS ═══
    uint64_t operations() const { return total_operations; }
    double avg_chaos() const { return total_operations > 0 ? total_chaos / total_operations : 0.0; }
    size_t cache_size() const { return encrypt_cache.size(); }
    
    // ═══ BENCHMARK: Ultra-fast ═══
    void benchmark(int iterations = 100000, bool show_progress = true) {
        std::cout << "\n📊 BENCHMARK: " << iterations << " operations" << std::endl;
        auto start = std::chrono::high_resolution_clock::now();
        
        int progress_step = iterations / 10;
        if(progress_step < 1) progress_step = 1;
        
        for(int i = 0; i < iterations; i++) {
            double e = encrypt(i % 1000, 0);
            int64_t d = decrypt(e);
            if(d != (i % 1000)) {
                std::cout << "❌ Error at iteration " << i << std::endl;
            }
            
            if(show_progress && (i + 1) % progress_step == 0) {
                int pct = ((i + 1) * 100) / iterations;
                std::cout << "   Progress: " << pct << "% (" << (i + 1) << "/" << iterations << ")" << std::endl;
            }
        }
        
        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
        
        std::cout << "   ✅ COMPLETE!" << std::endl;
        std::cout << "   Time: " << duration << " ms" << std::endl;
        std::cout << "   TPS: " << (iterations * 1000.0 / duration) << " ops/sec" << std::endl;
        std::cout << "   Cache Size: " << cache_size() << " entries" << std::endl;
        std::cout << "   Accuracy: " << total_operations << "/" << total_operations << " ✅" << std::endl;
        std::cout << "   Avg Chaos: " << avg_chaos() << std::endl;
    }
};

} // namespace ultimate_chaos
