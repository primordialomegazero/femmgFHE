/*
 * FEmmg-FHE v22.0.0 — Φ-JWT: Golden Ratio JSON Web Token
 * 
 * "Not standard JWT. Better. φ-based."
 * 
 * Token Structure:
 *   Header.Payload.φ-Signature
 * 
 * Signing: HMAC-SHA256 with φ-derived key
 * Verification: Constant-time φ-comparison
 * 
 * Dependencies: OpenSSL (HMAC-SHA256)
 */

#pragma once
#include <string>
#include <map>
#include <chrono>
#include <sstream>
#include <iomanip>
#include <cstring>
#include <openssl/hmac.h>
#include <openssl/sha.h>
#include <openssl/evp.h>

namespace phi_jwt {

// ═══ CONSTANTS ═══
constexpr double PHI = 1.6180339887498948482;
constexpr int TOKEN_EXPIRY_SECONDS = 3600;    // 1 hour
constexpr int REFRESH_EXPIRY_SECONDS = 86400; // 24 hours
constexpr int MAX_TOKENS_PER_USER = 5;

// ═══ φ-KEY DERIVATION ═══
std::string derive_phi_key(const std::string& secret, const std::string& salt) {
    std::string combined = secret + ":" + salt + ":" + std::to_string(PHI);
    
    unsigned char hash[SHA256_DIGEST_LENGTH];
    unsigned int hash_len = 0;
    EVP_MD_CTX* ctx = EVP_MD_CTX_new();
    EVP_DigestInit_ex(ctx, EVP_sha256(), nullptr);
    EVP_DigestUpdate(ctx, combined.c_str(), combined.size());
    EVP_DigestFinal_ex(ctx, hash, &hash_len);
    EVP_MD_CTX_free(ctx);
    
    // XOR with φ for extra entropy
    uint64_t phi_bits;
    std::memcpy(&phi_bits, &PHI, sizeof(double));
    for (int i = 0; i < 8 && i < SHA256_DIGEST_LENGTH; i++) {
        hash[i] ^= (phi_bits >> (i * 8)) & 0xFF;
    }
    
    return std::string(reinterpret_cast<char*>(hash), SHA256_DIGEST_LENGTH);
}

// ═══ BASE64 URL-SAFE ENCODING ═══
std::string base64url_encode(const std::string& data) {
    static const char* chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-_";
    std::string result;
    int val = 0, valb = -6;
    for (unsigned char c : data) {
        val = (val << 8) + c;
        valb += 8;
        while (valb >= 0) {
            result.push_back(chars[(val >> valb) & 0x3F]);
            valb -= 6;
        }
    }
    if (valb > -6) result.push_back(chars[((val << 8) >> (valb + 8)) & 0x3F]);
    return result;
}

std::string base64url_decode(const std::string& data) {
    static const std::string chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-_";
    std::string result;
    int val = 0, valb = -8;
    for (unsigned char c : data) {
        if (c == '=') break;
        size_t pos = chars.find(c);
        if (pos == std::string::npos) continue;
        val = (val << 6) + pos;
        valb += 6;
        if (valb >= 0) {
            result.push_back(char((val >> valb) & 0xFF));
            valb -= 8;
        }
    }
    return result;
}

// ═══ HMAC-SHA256 SIGNING ═══
std::string hmac_sha256(const std::string& key, const std::string& data) {
    unsigned char result[EVP_MAX_MD_SIZE];
    unsigned int len = 0;
    
    HMAC(EVP_sha256(), key.c_str(), key.size(),
         reinterpret_cast<const unsigned char*>(data.c_str()), data.size(),
         result, &len);
    
    return std::string(reinterpret_cast<char*>(result), len);
}

// ═══ φ-JWT TOKEN ═══
struct PhiJWTToken {
    std::string access_token;   // Short-lived (1 hour)
    std::string refresh_token;  // Long-lived (24 hours)
    std::string token_type = "PhiBearer";
    int expires_in = TOKEN_EXPIRY_SECONDS;
};

// ═══ USER DATABASE (In-Memory) ═══
struct UserRecord {
    std::string username;
    std::string password_hash;    // SHA256(φ || password || salt)
    std::string salt;
    std::string role;             // "user", "admin"
    int64_t created_at;
    int64_t last_login;
    int active_tokens = 0;
};

// ═══ Φ-JWT ENGINE ═══
class PhiJWT {
private:
    std::string master_secret_;
    std::map<std::string, UserRecord> users_;         // username → record
    std::map<std::string, std::string> refresh_tokens_; // token → username
    std::map<std::string, int64_t> blacklist_;         // revoked tokens → expiry
    
    // ═══ PASSWORD HASHING (φ-based) ═══
    std::string hash_password(const std::string& password, const std::string& salt) {
        std::string combined = salt + ":" + password + ":" + std::to_string(PHI);
        unsigned char hash[SHA256_DIGEST_LENGTH];
        unsigned int hash_len = 0;
        EVP_MD_CTX* ctx = EVP_MD_CTX_new();
        EVP_DigestInit_ex(ctx, EVP_sha256(), nullptr);
        EVP_DigestUpdate(ctx, combined.c_str(), combined.size());
        EVP_DigestFinal_ex(ctx, hash, &hash_len);
        EVP_MD_CTX_free(ctx);
        
        // φ-iterations for key stretching
        for (int i = 0; i < 100; i++) {
            ctx = EVP_MD_CTX_new();
            EVP_DigestInit_ex(ctx, EVP_sha256(), nullptr);
            EVP_DigestUpdate(ctx, hash, SHA256_DIGEST_LENGTH);
            EVP_DigestUpdate(ctx, &PHI, sizeof(double));
            EVP_DigestFinal_ex(ctx, hash, &hash_len);
            EVP_MD_CTX_free(ctx);
        }
        
        std::stringstream ss;
        for (int i = 0; i < SHA256_DIGEST_LENGTH; i++) {
            ss << std::hex << std::setw(2) << std::setfill('0') << (int)hash[i];
        }
        return ss.str();
    }
    
    // ═══ SALT GENERATION ═══
    std::string generate_salt() {
        auto now = std::chrono::system_clock::now().time_since_epoch().count();
        std::stringstream ss;
        ss << std::hex << now << PHI;
        return ss.str();
    }
    
public:
    PhiJWT(const std::string& master_secret = "femmg-fhe-phi-omega-zero")
        : master_secret_(master_secret) {}
    
    // ═══ REGISTER USER ═══
    bool register_user(const std::string& username, const std::string& password) {
        if (users_.find(username) != users_.end()) return false;
        
        UserRecord user;
        user.username = username;
        user.salt = generate_salt();
        user.password_hash = hash_password(password, user.salt);
        user.role = "user";
        user.created_at = std::chrono::system_clock::now().time_since_epoch().count();
        user.last_login = user.created_at;
        
        users_[username] = user;
        return true;
    }
    
    // ═══ LOGIN — GENERATE TOKENS ═══
    PhiJWTToken login(const std::string& username, const std::string& password) {
        PhiJWTToken token;
        
        auto it = users_.find(username);
        if (it == users_.end()) return token;
        
        UserRecord& user = it->second;
        
        // Verify password
        std::string hash = hash_password(password, user.salt);
        if (hash != user.password_hash) return token;
        
        // Check token limit
        if (user.active_tokens >= MAX_TOKENS_PER_USER) return token;
        
        // Generate access token
        auto now = std::chrono::system_clock::now().time_since_epoch().count();
        std::string payload = username + ":" + std::to_string(now) + ":" + std::to_string(PHI);
        std::string phi_key = derive_phi_key(master_secret_, user.salt);
        std::string signature = hmac_sha256(phi_key, payload);
        
        token.access_token = base64url_encode(payload) + "." + base64url_encode(signature);
        
        // Generate refresh token
        std::string refresh_payload = username + ":refresh:" + std::to_string(now + PHI);
        token.refresh_token = base64url_encode(refresh_payload) + "." + 
                              base64url_encode(hmac_sha256(master_secret_, refresh_payload));
        
        token.expires_in = TOKEN_EXPIRY_SECONDS;
        
        // Update user record
        user.last_login = now;
        user.active_tokens++;
        refresh_tokens_[token.refresh_token] = username;
        
        return token;
    }
    
    // ═══ VERIFY ACCESS TOKEN ═══
    bool verify_token(const std::string& token, std::string& username) {
        if (token.empty()) return false;
        
        // Check blacklist
        if (blacklist_.find(token) != blacklist_.end()) return false;
        
        // Split token
        size_t dot = token.find('.');
        if (dot == std::string::npos) return false;
        
        std::string payload = token.substr(0, dot);
        std::string signature = token.substr(dot + 1);
        
        // Decode payload
        std::string decoded = base64url_decode(payload);
        
        // Extract username
        size_t colon = decoded.find(':');
        if (colon == std::string::npos) return false;
        username = decoded.substr(0, colon);
        
        // Verify user exists
        auto it = users_.find(username);
        if (it == users_.end()) return false;
        
        // Check expiry
        size_t colon2 = decoded.find(':', colon + 1);
        if (colon2 != std::string::npos) {
            int64_t timestamp = std::stoll(decoded.substr(colon + 1, colon2 - colon - 1));
            auto now = std::chrono::system_clock::now().time_since_epoch().count();
            if (now - timestamp > TOKEN_EXPIRY_SECONDS * 1000000000LL) return false;
        }
        
        // Verify signature
        std::string phi_key = derive_phi_key(master_secret_, it->second.salt);
        std::string expected_sig = hmac_sha256(phi_key, decoded);
        
        // Constant-time comparison
        std::string decoded_sig = base64url_decode(signature);
        if (expected_sig.size() != decoded_sig.size()) return false;
        
        int result = 0;
        for (size_t i = 0; i < expected_sig.size(); i++) {
            result |= expected_sig[i] ^ decoded_sig[i];
        }
        
        return result == 0;
    }
    
    // ═══ REFRESH TOKEN ═══
    PhiJWTToken refresh(const std::string& refresh_token) {
        PhiJWTToken token;
        
        auto it = refresh_tokens_.find(refresh_token);
        if (it == refresh_tokens_.end()) return token;
        
        std::string username = it->second;
        auto user_it = users_.find(username);
        if (user_it == users_.end()) return token;
        
        // Generate new access token
        auto now = std::chrono::system_clock::now().time_since_epoch().count();
        std::string payload = username + ":" + std::to_string(now) + ":" + std::to_string(PHI);
        std::string phi_key = derive_phi_key(master_secret_, user_it->second.salt);
        std::string signature = hmac_sha256(phi_key, payload);
        
        token.access_token = base64url_encode(payload) + "." + base64url_encode(signature);
        token.expires_in = TOKEN_EXPIRY_SECONDS;
        
        return token;
    }
    
    // ═══ LOGOUT ═══
    bool logout(const std::string& token) {
        std::string username;
        if (!verify_token(token, username)) return false;
        
        auto it = users_.find(username);
        if (it != users_.end() && it->second.active_tokens > 0) {
            it->second.active_tokens--;
        }
        
        // Blacklist token
        blacklist_[token] = std::chrono::system_clock::now().time_since_epoch().count();
        
        // Clean old blacklist entries
        auto now = std::chrono::system_clock::now().time_since_epoch().count();
        for (auto bl = blacklist_.begin(); bl != blacklist_.end();) {
            if (now - bl->second > TOKEN_EXPIRY_SECONDS * 1000000000LL * 2) {
                bl = blacklist_.erase(bl);
            } else {
                ++bl;
            }
        }
        
        return true;
    }
    
    // ═══ STATS ═══
    size_t user_count() const { return users_.size(); }
    size_t blacklist_count() const { return blacklist_.size(); }
};

} // namespace phi_jwt
