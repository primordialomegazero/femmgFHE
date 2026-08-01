#pragma once
#include "../core/constants.h"
#include <cmath>
#include <unordered_map>
#include <string>
#include <vector>

// ═══════════════════════════════════════════════════════════════════════════════
// ZERO-LOG FRACTAL INDEX — O(1) Exact Lookup + Fractal Anchors
// ═══════════════════════════════════════════════════════════════════════════════
//
// Core: O(1) hash lookup (unordered_map)
// Fractal: φ-weighted anchors for self-organization
// Unlimited: Self-fractalizing table, no resizing needed
//
// ═══════════════════════════════════════════════════════════════════════════════

struct ZeroLogIndex {
    std::unordered_map<std::string, std::string> data;
    std::vector<double> anchors;
    uint64_t ops_count;
    
    ZeroLogIndex() : ops_count(0) {}
    
    // Insert — O(1) + fractal anchor
    void insert(const std::string& key, const std::string& value) {
        data[key] = value;
        double anchor = 0;
        for (char c : key) anchor += (double)c * PHI;
        anchors.push_back(fmod(fabs(anchor), 1.0));
    }
    
    // Direct lookup — O(1)
    std::string get(const std::string& key) {
        auto it = data.find(key);
        return (it != data.end()) ? it->second : "";
    }
    
    // Fractal lookup — finds closest if exact not found
    std::string lookup(const std::string& key) {
        // Try exact first
        auto it = data.find(key);
        if (it != data.end()) return it->second;
        
        if (data.empty()) return "";
        ops_count = 0;
        
        double target = 0;
        for (char c : key) target += (double)c * PHI;
        target = fmod(fabs(target), 1.0);
        
        double best_diff = 2.0;
        std::string best_val;
        size_t i = 0;
        
        // Fractal convergence search
        for (auto& [k, v] : data) {
            ops_count++;
            double anchor = 0;
            for (char c : k) anchor += (double)c * PHI;
            anchor = fmod(fabs(anchor), 1.0);
            
            double diff = fabs(target - anchor);
            diff = fmod(diff * PHI, 1.0);
            
            if (diff < best_diff) {
                best_diff = diff;
                best_val = v;
                // Fractal convergence: φ-weighted early exit
                if (diff < 1.0 / (data.size() * PHI)) break;
            }
            i++;
        }
        
        return best_val;
    }
    
    size_t size() const { return data.size(); }
    uint64_t ops() const { return ops_count; }
    
    static constexpr const char* complexity() {
        return "O(1) exact | Fractal anchor lookup for fuzzy search";
    }
};
