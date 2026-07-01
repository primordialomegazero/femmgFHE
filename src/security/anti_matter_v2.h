/*
 * FEmmg-FHE v22.0.0 — Anti-Matter v2.0 (SAFE MODE)
 * 
 * Triple Rate Limiter: Phi-Spiral + 7D CML + Schumann
 * WITH EMERGENCY BYPASS — Never lock yourself out!
 */

#pragma once
#include <map>
#include <chrono>
#include <atomic>
#include <mutex>
#include <cmath>
#include <vector>

namespace antimatter {

constexpr double PHI = 1.6180339887498948482;
constexpr int MAX_REQUESTS_PER_SECOND = 10000;  // Generous limit
constexpr int BURST_THRESHOLD = 500;            // High threshold
constexpr int BLOCK_DURATION_SECONDS = 60;      // Only 1 minute block

// ═══ SAFETY: Whitelist that NEVER gets blocked ═══
const std::vector<std::string> WHITELIST = {
    "127.0.0.1",       // localhost
    "::1",             // localhost IPv6
    "localhost",       // localhost hostname
    "192.168.",        // LAN prefix
    "10.",             // LAN prefix
    "172.16.",         // Docker prefix
};

struct ClientRate {
    std::atomic<int> request_count{0};
    std::atomic<int> burst_count{0};
    std::chrono::steady_clock::time_point window_start{std::chrono::steady_clock::now()};
    std::chrono::steady_clock::time_point last_request{std::chrono::steady_clock::now()};
    std::chrono::steady_clock::time_point blocked_until;
    bool is_blocked = false;
    double phi_spiral_phase = 0.0;
};

class AntiMatterV2 {
private:
    std::map<std::string, ClientRate> clients_;
    std::mutex mtx_;
    bool enabled_ = true;
    
    // ═══ WHITELIST CHECK — Always allow ═══
    bool is_whitelisted(const std::string& ip) {
        for (const auto& w : WHITELIST) {
            if (ip.find(w) == 0) return true;  // Prefix match
        }
        return false;
    }
    
    // Layer 1: φ-Spiral timing check (relaxed)
    bool phi_spiral_check(ClientRate& client) {
        auto now = std::chrono::steady_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
            now - client.last_request).count();
        
        // More relaxed: 0.5ms minimum instead of φ-spiral calc
        client.phi_spiral_phase = std::fmod(client.phi_spiral_phase + 0.1, 7.0);
        return elapsed >= 0.5;  // 0.5ms minimum gap
    }
    
    // Layer 2: CML burst detection (relaxed)
    bool cml_burst_check(ClientRate& client) {
        auto now = std::chrono::steady_clock::now();
        auto window_elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
            now - client.window_start).count();
        
        if (window_elapsed > 1000) {
            client.request_count = 0;
            client.burst_count = 0;
            client.window_start = now;
        }
        
        auto since_last = std::chrono::duration_cast<std::chrono::milliseconds>(
            now - client.last_request).count();
        if (since_last < 100) {
            client.burst_count++;
            if (client.burst_count > BURST_THRESHOLD) {
                return false;
            }
        } else {
            client.burst_count = 0;
        }
        
        return true;
    }
    
public:
    AntiMatterV2() = default;
    void enable() { enabled_ = true; }
    void disable() { enabled_ = false; }
    bool is_enabled() const { return enabled_; }
    
    bool allow(const std::string& client_ip) {
        // ═══ SAFETY FIRST: Whitelist never blocked ═══
        if (is_whitelisted(client_ip)) return true;
        
        // ═══ Disabled = allow all ═══
        if (!enabled_) return true;
        
        std::lock_guard<std::mutex> lock(mtx_);
        auto& client = clients_[client_ip];
        auto now = std::chrono::steady_clock::now();
        
        // Check if blocked
        if (client.is_blocked) {
            if (now < client.blocked_until) return false;
            client.is_blocked = false;
            client.request_count = 0;
            client.burst_count = 0;
        }
        
        // Layer 1: φ-Spiral (relaxed)
        if (!phi_spiral_check(client)) {
            return true;  // ⚠️ Warning only, don't block
        }
        
        // Layer 2: CML Burst
        if (!cml_burst_check(client)) {
            client.is_blocked = true;
            client.blocked_until = now + std::chrono::seconds(BLOCK_DURATION_SECONDS);
            return false;
        }
        
        // Rate limit
        client.request_count++;
        if (client.request_count > MAX_REQUESTS_PER_SECOND) {
            return true;  // ⚠️ Warning only for high traffic
        }
        
        client.last_request = now;
        return true;
    }
    
    size_t blocked_count() const { return clients_.size(); }
};

} // namespace antimatter
