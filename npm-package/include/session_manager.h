/*
 * FEmmg-FHE v22.0.0 — Session Manager
 * 
 * Secure session management with:
 * - Token-based sessions (not plain client_id)
 * - Auto-expiration (1 hour idle, 24 hour max)
 * - Concurrent session limits
 * - Session revocation
 */

#pragma once
#include <string>
#include <map>
#include <chrono>
#include <mutex>
#include <random>
#include <atomic>

namespace session_manager {

constexpr int SESSION_IDLE_TIMEOUT = 3600;   // 1 hour
constexpr int SESSION_MAX_LIFETIME = 86400;  // 24 hours
constexpr int MAX_SESSIONS_PER_USER = 3;
constexpr int TOKEN_LENGTH = 64;

struct Session {
    std::string session_token;
    std::string client_id;
    int64_t created_at;
    int64_t last_activity;
    bool active = true;
};

class SessionManager {
private:
    std::map<std::string, Session> sessions_;      // token → session
    std::map<std::string, int> user_sessions_;     // client_id → count
    std::mutex mtx_;
    std::mt19937_64 rng_;
    
    // ═══ GENERATE SECURE TOKEN ═══
    std::string generate_token() {
        static const char chars[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789-_";
        std::uniform_int_distribution<int> dist(0, 63);
        
        std::string token;
        token.reserve(TOKEN_LENGTH);
        for (int i = 0; i < TOKEN_LENGTH; i++) {
            token += chars[dist(rng_)];
        }
        return token;
    }
    
    int64_t now() const {
        return std::chrono::system_clock::now().time_since_epoch().count() / 1000000000;
    }
    
public:
    SessionManager() : rng_(std::random_device{}()) {}
    
    // ═══ CREATE SESSION ═══
    std::string create_session(const std::string& client_id) {
        std::lock_guard<std::mutex> lock(mtx_);
        
        // Check concurrent limit
        if (user_sessions_[client_id] >= MAX_SESSIONS_PER_USER) {
            return "";  // Too many sessions
        }
        
        // Create session
        Session session;
        session.session_token = generate_token();
        session.client_id = client_id;
        session.created_at = now();
        session.last_activity = session.created_at;
        
        sessions_[session.session_token] = session;
        user_sessions_[client_id]++;
        
        return session.session_token;
    }
    
    // ═══ VALIDATE SESSION ═══
    bool validate_session(const std::string& token, std::string& client_id) {
        std::lock_guard<std::mutex> lock(mtx_);
        
        auto it = sessions_.find(token);
        if (it == sessions_.end()) return false;
        
        Session& session = it->second;
        if (!session.active) return false;
        
        auto current_time = now();
        
        // Check idle timeout
        if (current_time - session.last_activity > SESSION_IDLE_TIMEOUT) {
            session.active = false;
            user_sessions_[session.client_id]--;
            return false;
        }
        
        // Check max lifetime
        if (current_time - session.created_at > SESSION_MAX_LIFETIME) {
            session.active = false;
            user_sessions_[session.client_id]--;
            return false;
        }
        
        // Update activity
        session.last_activity = current_time;
        client_id = session.client_id;
        return true;
    }
    
    // ═══ REVOKE SESSION ═══
    bool revoke_session(const std::string& token) {
        std::lock_guard<std::mutex> lock(mtx_);
        
        auto it = sessions_.find(token);
        if (it == sessions_.end()) return false;
        
        it->second.active = false;
        user_sessions_[it->second.client_id]--;
        return true;
    }
    
    // ═══ CLEANUP EXPIRED SESSIONS ═══
    void cleanup() {
        std::lock_guard<std::mutex> lock(mtx_);
        auto current_time = now();
        
        for (auto it = sessions_.begin(); it != sessions_.end();) {
            if (!it->second.active || 
                current_time - it->second.last_activity > SESSION_IDLE_TIMEOUT ||
                current_time - it->second.created_at > SESSION_MAX_LIFETIME) {
                user_sessions_[it->second.client_id]--;
                it = sessions_.erase(it);
            } else {
                ++it;
            }
        }
    }
    
    // ═══ STATS ═══
    size_t active_sessions() const { return sessions_.size(); }
    size_t session_count(const std::string& client_id) const {
        auto it = user_sessions_.find(client_id);
        return it != user_sessions_.end() ? it->second : 0;
    }
};

} // namespace session_manager
