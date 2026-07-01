/*
 * FEmmg-FHE v22.0.0 — API Authentication
 * 
 * Simple API key authentication for session protection.
 * 
 * Usage:
 *   curl -X POST http://localhost:8092/ \
 *     -H "Authorization: Bearer YOUR_API_KEY" \
 *     -d '{"action":"fhe_encrypt","plaintext":42}'
 */

#pragma once
#include <string>
#include <map>
#include <mutex>
#include <array>
#include <openssl/sha.h>

class APIAuth {
private:
    std::map<std::string, std::string> api_keys_;  // client_id → api_key
    std::mutex mtx_;
    bool enabled_ = false;
    
    std::string sha256(const std::string& input) {
        unsigned char hash[SHA256_DIGEST_LENGTH];
        SHA256_CTX ctx;
        SHA256_Init(&ctx);
        SHA256_Update(&ctx, input.c_str(), input.size());
        SHA256_Final(hash, &ctx);
        
        std::string result;
        for (int i = 0; i < SHA256_DIGEST_LENGTH; i++) {
            char buf[3];
            snprintf(buf, sizeof(buf), "%02x", hash[i]);
            result += buf;
        }
        return result;
    }
    
public:
    APIAuth() = default;
    
    // Enable authentication
    void enable() { enabled_ = true; }
    void disable() { enabled_ = false; }
    bool is_enabled() const { return enabled_; }
    
    // Register a client with API key
    std::string register_client(const std::string& client_id) {
        std::lock_guard<std::mutex> lock(mtx_);
        
        // Generate API key from client_id + hash
        std::string api_key = sha256(client_id + "femmg-fhe-salt");
        api_keys_[client_id] = api_key;
        
        return api_key;
    }
    
    // Verify API key
    bool verify(const std::string& client_id, const std::string& api_key) {
        if (!enabled_) return true;  // Allow all if auth is disabled
        
        std::lock_guard<std::mutex> lock(mtx_);
        auto it = api_keys_.find(client_id);
        if (it == api_keys_.end()) return false;
        
        return it->second == api_key;
    }
    
    // Remove client
    void unregister(const std::string& client_id) {
        std::lock_guard<std::mutex> lock(mtx_);
        api_keys_.erase(client_id);
    }
};
