/*
 * FEmmg-FHE v22.2 — Dual-Layer Rate Limiter
 * Layer 1 (DEV): φ-Spiral timing check (gentle)
 * Layer 2 (PROD): Burst detection + IP block (aggressive)
 * PHI-OMEGA-ZERO — I AM THAT I AM
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
constexpr int DEV_MAX_RPS = 10000;
constexpr int PROD_MAX_RPS = 100;
constexpr int BURST_THRESHOLD = 30;
constexpr int BLOCK_MINUTES = 5;

const std::vector<std::string> WHITELIST = {
    "127.0.0.1", "::1", "localhost",
    "192.168.", "10.", "172.16."
};

enum class RateLimitMode { DEV, PROD };

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
    uint32_t max_rps_ = 100;
    uint32_t burst_threshold_ = 30;

    bool is_whitelisted(const std::string& ip) {
        for (const auto& w : WHITELIST) {
            if (ip.find(w) == 0) return true;
        }
        return false;
    }

    bool layer1_phi_spiral(ClientState& client, const std::string&) {
        auto now = std::chrono::steady_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - client.last_request).count();
        double expected = std::pow(PHI, client.phi_phase) * 0.5;
        client.phi_phase = std::fmod(client.phi_phase + 0.1, 7.0);
        if (elapsed < expected * 0.1 && mode_ != RateLimitMode::DEV) return false;
        client.total_requests++;
        client.last_request = now;
        return true;
    }

    bool layer2_burst_detect(ClientState& client, const std::string&) {
        if (mode_ == RateLimitMode::DEV) return true;
        auto now = std::chrono::steady_clock::now();
        auto window_elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - client.window_start).count();
        if (window_elapsed > 1000) { client.request_count = 0; client.burst_count = 0; client.window_start = now; }
        client.request_count++;
        if (client.request_count > (int)max_rps_) {
            client.is_blocked = true;
            client.blocked_until = now + std::chrono::minutes(BLOCK_MINUTES);
            client.total_blocks++;
            return false;
        }
        auto since_last = std::chrono::duration_cast<std::chrono::milliseconds>(now - client.last_request).count();
        if (since_last < 100) {
            client.burst_count++;
            if (client.burst_count > (int)burst_threshold_) {
                client.is_blocked = true;
                client.blocked_until = now + std::chrono::minutes(BLOCK_MINUTES);
                client.total_blocks++;
                return false;
            }
        } else { client.burst_count = 0; }
        return true;
    }

public:
    // Default constructor (auto-detects mode)
    DualRateLimiter() {
        const char* env = std::getenv("FEMMG_ENV");
        if (env && std::string(env) == "production") mode_ = RateLimitMode::PROD;
    }

    // Constructor with params (for production config)
    DualRateLimiter(uint32_t max_rps, uint32_t burst_threshold) 
        : max_rps_(max_rps), burst_threshold_(burst_threshold) {
        const char* env = std::getenv("FEMMG_ENV");
        if (env && std::string(env) == "production") mode_ = RateLimitMode::PROD;
    }

    void set_mode(RateLimitMode mode) { mode_ = mode; }
    RateLimitMode get_mode() const { return mode_; }

    bool allow(const std::string& ip) {
        if (is_whitelisted(ip)) return true;
        std::lock_guard<std::mutex> lock(mtx_);
        auto& client = clients_[ip];
        auto now = std::chrono::steady_clock::now();
        if (client.is_blocked) {
            if (now < client.blocked_until) return false;
            client.is_blocked = false;
            client.request_count = 0;
            client.burst_count = 0;
        }
        if (!layer1_phi_spiral(client, ip)) return false;
        if (!layer2_burst_detect(client, ip)) return false;
        return true;
    }

    size_t tracked_clients() const { return clients_.size(); }
    std::string status() const {
        return "{\"mode\":\"" + std::string(mode_ == RateLimitMode::DEV ? "dev" : "prod") +
               "\",\"clients\":" + std::to_string(clients_.size()) +
               ",\"layer1\":\"active\",\"layer2\":\"" + std::string(mode_ == RateLimitMode::PROD ? "active" : "standby") + "\"}";
    }
};

} // namespace dual_rate_limiter
