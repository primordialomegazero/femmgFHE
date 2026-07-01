/*
 * FEmmg-FHE v22.1 — Dual-Layer Rate Limiter
 * 
 * "Like Integer-Float Merge. Two layers, one purpose."
 * 
 * Architecture:
 *   Layer 1 (DEV):  Always ON — φ-Spiral timing check (gentle)
 *   Layer 2 (PROD): Auto-triggered — Burst detection + IP block (aggressive)
 * 
 * Auto-detection:
 *   - FEMMG_ENV=production → Both layers active
 *   - Default (dev) → Layer 1 only (never blocks, just warns)
 * 
 * "Dev never blocked. Prod fully protected."
 */

#pragma once
#include <map>
#include <chrono>
#include <mutex>
#include <string>
#include <vector>
#include <cmath>
#include <cstdlib>

namespace dual_rate_limiter {

constexpr double PHI = 1.6180339887498948482;
constexpr int DEV_MAX_RPS = 10000;      // Dev: very generous
constexpr int PROD_MAX_RPS = 100;       // Prod: strict
constexpr int BURST_THRESHOLD = 30;     // Requests in <100ms = burst
constexpr int BLOCK_MINUTES = 5;        // Block duration

// ═══ WHITELIST — Never blocked ═══
const std::vector<std::string> WHITELIST = {
    "127.0.0.1", "::1", "localhost",
    "192.168.", "10.", "172.16."
};

enum class RateLimitMode {
    DEV,     // Layer 1 only (gentle)
    PROD     // Both layers (aggressive)
};

struct ClientState {
    int request_count = 0;
    int burst_count = 0;
    std::chrono::steady_clock::time_point window_start{std::chrono::steady_clock::now()};
    std::chrono::steady_clock::time_point last_request{std::chrono::steady_clock::now()};
    std::chrono::steady_clock::time_point blocked_until;
    bool is_blocked = false;
    double phi_phase = 0.0;
    int total_requests = 0;
    int total_blocks = 0;
};

class DualRateLimiter {
private:
    std::map<std::string, ClientState> clients_;
    std::mutex mtx_;
    RateLimitMode mode_ = RateLimitMode::DEV;
    
    bool is_whitelisted(const std::string& ip) {
        for (const auto& w : WHITELIST) {
            if (ip.find(w) == 0) return true;
        }
        return false;
    }
    
    // ═══ LAYER 1: φ-Spiral Timing (Always ON) ═══
    bool layer1_phi_spiral(ClientState& client, const std::string& /*ip*/) {
        auto now = std::chrono::steady_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
            now - client.last_request).count();
        
        // φ-spiral expected intervals: 1, 2, 3, 5, 8, 13, 21... ms
        double expected = std::pow(PHI, client.phi_phase) * 0.5;
        client.phi_phase = std::fmod(client.phi_phase + 0.1, 7.0);
        
        if (elapsed < expected * 0.1) {
            // Too fast — but in DEV mode, just warn
            if (mode_ == RateLimitMode::DEV) {
                client.total_requests++;
                client.last_request = now;
                return true;  // Allow but track
            }
            return false;  // PROD: reject
        }
        
        client.total_requests++;
        client.last_request = now;
        return true;
    }
    
    // ═══ LAYER 2: Burst Detection (PROD only) ═══
    bool layer2_burst_detect(ClientState& client, const std::string& /*ip*/) {
        if (mode_ == RateLimitMode::DEV) return true;  // Skip in dev
        
        auto now = std::chrono::steady_clock::now();
        
        // Reset window every second
        auto window_elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
            now - client.window_start).count();
        if (window_elapsed > 1000) {
            client.request_count = 0;
            client.burst_count = 0;
            client.window_start = now;
        }
        
        client.request_count++;
        
        // Check rate limit
        if (client.request_count > PROD_MAX_RPS) {
            client.is_blocked = true;
            client.blocked_until = now + std::chrono::minutes(BLOCK_MINUTES);
            client.total_blocks++;
            return false;
        }
        
        // Check burst
        auto since_last = std::chrono::duration_cast<std::chrono::milliseconds>(
            now - client.last_request).count();
        if (since_last < 100) {
            client.burst_count++;
            if (client.burst_count > BURST_THRESHOLD) {
                client.is_blocked = true;
                client.blocked_until = now + std::chrono::minutes(BLOCK_MINUTES);
                client.total_blocks++;
                return false;
            }
        } else {
            client.burst_count = 0;
        }
        
        return true;
    }
    
public:
    DualRateLimiter() {
        // Auto-detect mode
        const char* env = std::getenv("FEMMG_ENV");
        if (env && std::string(env) == "production") {
            mode_ = RateLimitMode::PROD;
        }
    }
    
    void set_mode(RateLimitMode mode) { mode_ = mode; }
    RateLimitMode get_mode() const { return mode_; }
    
    // ═══ MAIN CHECK ═══
    bool allow(const std::string& ip) {
        // Whitelist always passes
        if (is_whitelisted(ip)) return true;
        
        std::lock_guard<std::mutex> lock(mtx_);
        auto& client = clients_[ip];
        auto now = std::chrono::steady_clock::now();
        
        // Check if blocked
        if (client.is_blocked) {
            if (now < client.blocked_until) return false;
            client.is_blocked = false;
            client.request_count = 0;
            client.burst_count = 0;
        }
        
        // Layer 1: Always active
        if (!layer1_phi_spiral(client, ip)) return false;
        
        // Layer 2: PROD only
        if (!layer2_burst_detect(client, ip)) return false;
        
        return true;
    }
    
    // ═══ STATS ═══
    size_t tracked_clients() const { return clients_.size(); }
    
    std::string status() const {
        return "{"
               "\"mode\":\"" + std::string(mode_ == RateLimitMode::DEV ? "dev" : "prod") + "\","
               "\"clients\":" + std::to_string(clients_.size()) + ","
               "\"layer1\":\"active\","
               "\"layer2\":\"" + std::string(mode_ == RateLimitMode::PROD ? "active" : "standby") + "\""
               "}";
    }
};

} // namespace dual_rate_limiter
