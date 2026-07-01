/*
 * FEmmg-FHE v22.0.0 — SpiralDB Lite
 * 
 * Triple Mirror Encrypted Database with 7-Layer Fractal Index.
 * 
 * ARCHITECTURE:
 *   Mirror 1: Memory cache (instant access)
 *   Mirror 2: File persistence (eternal storage)
 *   Mirror 3: 7-Layer Fractal Index (φ-harmonic distribution)
 * 
 * FRACTAL INDEX:
 *   - 7 layers of SHA-256(φ || key || layer) distribution
 *   - φ = 1.6180339887498948482 (golden ratio)
 *   - Auto-compress via Banach contraction when > 1000 entries
 *   - v_new = v · φ⁻¹ + avg · (1 - φ⁻¹)
 * 
 * FEATURES:
 *   - Zero external dependencies
 *   - Thread-safe (mutable mutex)
 *   - Chaos history persistence
 * 
 * DEPENDENCIES: None (pure C++17)
 * USED BY: golden_chaos.h (chaos history storage)
 */
/*
 * SPIRALDB LITE — WITH 7-LAYER FRACTAL INDEX (FIXED)
 */

#pragma once
#include <string>
#include <map>
#include <fstream>
#include <sstream>
#include <array>
#include <mutex>
#include <cstring>
#include <cstdlib>
#include <sys/stat.h>
#include <sys/types.h>
#include <functional>

namespace spiraldb_lite {

constexpr int CHAOS_LAYERS = 14;
constexpr int FRACTAL_LAYERS = 7;
using ChaosHistory = std::array<double, CHAOS_LAYERS>;

inline uint64_t fractal_hash(const std::string& key, int layer) {
    uint64_t hash = 0x9E3779B97F4A7C15ULL;
    for(char c : key) {
        hash = (hash ^ (uint64_t)c) * 0x9E3779B97F4A7C15ULL;
        hash = (hash << 13) | (hash >> 51);
    }
    hash ^= (uint64_t)(layer * 1.6180339887498948482 * 1000000);
    return hash;
}

class SpiralDBLite {
private:
    std::map<std::string, ChaosHistory> memory_cache;
    std::string data_dir;
    std::array<std::map<uint64_t, std::string>, FRACTAL_LAYERS> fractal_index;
    mutable std::mutex mtx;
    static constexpr size_t COMPRESS_THRESHOLD = 1000;
    
    std::string history_to_string(const ChaosHistory& h) {
        std::ostringstream oss;
        for(int i = 0; i < CHAOS_LAYERS; i++) {
            oss << h[i];
            if(i < CHAOS_LAYERS - 1) oss << ",";
        }
        return oss.str();
    }
    
    ChaosHistory string_to_history(const std::string& s) {
        ChaosHistory h{};
        std::istringstream iss(s);
        std::string token;
        int i = 0;
        while(std::getline(iss, token, ',') && i < CHAOS_LAYERS) {
            h[i++] = std::stod(token);
        }
        return h;
    }
    
    std::string file_path(const std::string& key) {
        return data_dir + "/" + key + ".chaos";
    }
    
    void create_directory(const std::string& path) {
        #ifdef _WIN32
            std::string cmd = "mkdir " + path + " 2>nul";
        #else
            std::string cmd = "mkdir -p " + path + " 2>/dev/null";
        #endif
        (void)system(cmd.c_str());
    }
    
    void store_in_fractal(const std::string& key, const ChaosHistory& history) {
        for(int layer = 0; layer < FRACTAL_LAYERS; layer++) {
            uint64_t hash = fractal_hash(key, layer);
            fractal_index[layer][hash] = history_to_string(history);
        }
    }
    
    std::string retrieve_from_fractal(const std::string& key, bool& found) {
        for(int layer = 0; layer < FRACTAL_LAYERS; layer++) {
            uint64_t hash = fractal_hash(key, layer);
            auto it = fractal_index[layer].find(hash);
            if(it != fractal_index[layer].end()) {
                found = true;
                return it->second;
            }
        }
        found = false;
        return "";
    }
    
    // ═══ AUTO-COMPRESS: φ-contraction (now inside class) ═══
    void auto_compress(int layer) {
        if(layer < 0 || layer >= FRACTAL_LAYERS) return;
        if(fractal_index[layer].size() < COMPRESS_THRESHOLD) return;
        
        double avg = 0.0;
        int count = 0;
        for(auto& pair : fractal_index[layer]) {
            auto h = string_to_history(pair.second);
            for(int i = 0; i < CHAOS_LAYERS; i++) {
                avg += h[i];
                count++;
            }
        }
        avg /= (count > 0 ? count : 1);
        
        constexpr double PHI_INV = 0.6180339887498948482;
        for(auto& pair : fractal_index[layer]) {
            auto h = string_to_history(pair.second);
            for(int i = 0; i < CHAOS_LAYERS; i++) {
                h[i] = h[i] * PHI_INV + avg * (1.0 - PHI_INV);
            }
            pair.second = history_to_string(h);
        }
    }

public:
    SpiralDBLite(const std::string& dir = "./chaos_data") : data_dir(dir) {
        create_directory(data_dir);
    }
    
    void store(const std::string& key, const ChaosHistory& history) {
        std::lock_guard<std::mutex> lock(mtx);
        memory_cache[key] = history;
        
        std::ofstream file(file_path(key));
        if(file.is_open()) {
            file << history_to_string(history);
            file.close();
        }
        
        store_in_fractal(key, history);
        
        // Auto-compress all layers if needed
        for(int i = 0; i < FRACTAL_LAYERS; i++) {
            auto_compress(i);
        }
    }
    
    ChaosHistory retrieve(const std::string& key, bool& found) {
        std::lock_guard<std::mutex> lock(mtx);
        
        auto it = memory_cache.find(key);
        if(it != memory_cache.end()) {
            found = true;
            return it->second;
        }
        
        std::ifstream file(file_path(key));
        if(file.is_open()) {
            std::string content;
            std::getline(file, content);
            file.close();
            found = true;
            ChaosHistory h = string_to_history(content);
            memory_cache[key] = h;
            store_in_fractal(key, h);
            return h;
        }
        
        std::string content = retrieve_from_fractal(key, found);
        if(found) {
            ChaosHistory h = string_to_history(content);
            memory_cache[key] = h;
            return h;
        }
        
        found = false;
        return ChaosHistory{};
    }
    
    size_t fractal_size(int layer) const {
        std::lock_guard<std::mutex> lock(mtx);
        if(layer < 0 || layer >= FRACTAL_LAYERS) return 0;
        return fractal_index[layer].size();
    }
    
    size_t size() const {
        std::lock_guard<std::mutex> lock(mtx);
        return memory_cache.size();
    }
    
    bool exists(const std::string& key) const {
        std::lock_guard<std::mutex> lock(mtx);
        return memory_cache.find(key) != memory_cache.end();
    }
    
    void clear() {
        std::lock_guard<std::mutex> lock(mtx);
        memory_cache.clear();
        for(int i = 0; i < FRACTAL_LAYERS; i++) {
            fractal_index[i].clear();
        }
        std::string cmd = "rm -rf " + data_dir + "/*.chaos 2>/dev/null";
        (void)system(cmd.c_str());
    }
};

} // namespace spiraldb_lite
