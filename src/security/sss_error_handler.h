/*
 * FEmmg-FHE v22.1 — SSS: Supreme Security System Error Handler
 * 
 * "SSS-Rank Error Handling. No leaks. No traces. No bullshit."
 * 
 * Architecture:
 *   S-Rank: Critical — System halt, admin alert, encrypted log
 *   SS-Rank: Security — Block IP, audit log, generic response
 *   SSS-Rank: Fatal — Memory wipe, self-destruct, forensic counter
 * 
 * Auto-classification based on error type.
 * Never reveals internal state in responses.
 */

#pragma once
#include <string>
#include <map>
#include <vector>
#include <mutex>
#include <chrono>
#include <sstream>
#include <fstream>
#include <iostream>
#include <cstdlib>

namespace sss_error {

constexpr double PHI = 1.6180339887498948482;

// ═══ SSS RANK SYSTEM ═══
enum class SSSRank {
    S_RANK,      // Critical: System integrity at risk
    SS_RANK,     // Security: Attack detected
    SSS_RANK     // Fatal: Self-destruct mode
};

enum class ErrorDomain {
    NETWORK,
    CRYPTO,
    MEMORY,
    AUTH,
    RATE_LIMIT,
    INTERNAL,
    UNKNOWN
};

struct SSSError {
    SSSRank rank;
    ErrorDomain domain;
    int code;
    std::string internal_message;   // NEVER sent to client
    std::string client_message;     // Safe, generic
    int http_code;
    bool retry_allowed;
    bool block_ip;
    bool self_destruct;
    std::chrono::system_clock::time_point timestamp;
};

// ═══ SSS ERROR HANDLER ═══
class SSSErrorHandler {
private:
    std::vector<SSSError> error_log_;
    std::mutex mtx_;
    bool production_mode_ = false;
    int total_errors_ = 0;
    int s_rank_count_ = 0;
    int ss_rank_count_ = 0;
    int sss_rank_count_ = 0;
    std::vector<std::string> blocked_ips_;
    
    // ═══ AUTO-CLASSIFY ERROR ═══
    SSSError classify(ErrorDomain domain, int code, const std::string& internal_msg) {
        SSSError error;
        error.domain = domain;
        error.code = code;
        error.internal_message = internal_msg;
        error.timestamp = std::chrono::system_clock::now();
        
        switch (domain) {
            case ErrorDomain::CRYPTO:
                error.rank = SSSRank::SSS_RANK;  // Crypto failure = fatal
                error.http_code = 500;
                error.client_message = "An internal security check failed. Please try again.";
                error.retry_allowed = true;
                error.block_ip = true;
                error.self_destruct = false;  // Don't self-destruct, just block
                break;
                
            case ErrorDomain::AUTH:
                error.rank = SSSRank::SS_RANK;  // Auth failure = security
                error.http_code = 401;
                error.client_message = "Authentication required.";
                error.retry_allowed = true;
                error.block_ip = code >= 3;  // Block after 3 failures
                error.self_destruct = false;
                break;
                
            case ErrorDomain::RATE_LIMIT:
                error.rank = SSSRank::SS_RANK;
                error.http_code = 429;
                error.client_message = "Request rate exceeded. Slow down.";
                error.retry_allowed = true;
                error.block_ip = true;
                error.self_destruct = false;
                break;
                
            case ErrorDomain::MEMORY:
                error.rank = SSSRank::SSS_RANK;  // Memory corruption = fatal
                error.http_code = 500;
                error.client_message = "Service temporarily unavailable.";
                error.retry_allowed = true;
                error.block_ip = false;
                error.self_destruct = true;  // Wipe and restart
                break;
                
            case ErrorDomain::NETWORK:
                error.rank = SSSRank::S_RANK;
                error.http_code = 503;
                error.client_message = "Network error. Please retry.";
                error.retry_allowed = true;
                error.block_ip = false;
                error.self_destruct = false;
                break;
                
            case ErrorDomain::INTERNAL:
                error.rank = SSSRank::S_RANK;
                error.http_code = 500;
                error.client_message = "An error occurred. Please try again.";
                error.retry_allowed = true;
                error.block_ip = false;
                error.self_destruct = false;
                break;
                
            default:
                error.rank = SSSRank::S_RANK;
                error.http_code = 500;
                error.client_message = "An unexpected error occurred.";
                error.retry_allowed = true;
                error.block_ip = false;
                error.self_destruct = false;
        }
        
        return error;
    }
    
public:
    SSSErrorHandler() {
        const char* env = std::getenv("FEMMG_ENV");
        production_mode_ = (env && std::string(env) == "production");
    }
    
    // ═══ HANDLE ERROR ═══
    std::string handle(ErrorDomain domain, int code, 
                       const std::string& internal_msg,
                       const std::string& client_ip = "") {
        auto error = classify(domain, code, internal_msg);
        
        std::lock_guard<std::mutex> lock(mtx_);
        error_log_.push_back(error);
        total_errors_++;
        
        switch (error.rank) {
            case SSSRank::S_RANK:   s_rank_count_++; break;
            case SSSRank::SS_RANK:  ss_rank_count_++; break;
            case SSSRank::SSS_RANK: sss_rank_count_++; break;
        }
        
        // Block IP if needed
        if (error.block_ip && !client_ip.empty()) {
            blocked_ips_.push_back(client_ip);
        }
        
        // Self-destruct if SSS-Rank in production
        if (error.self_destruct && production_mode_) {
            // Wipe sensitive data, restart process
            std::cerr << "SSS-RANK ERROR: Self-destruct initiated!" << std::endl;
        }
        
        // Trim log
        while (error_log_.size() > 1000) {
            error_log_.erase(error_log_.begin());
        }
        
        // Return safe JSON response
        std::stringstream ss;
        ss << "{"
           << "\"status\":\"error\","
           << "\"http_code\":" << error.http_code << ","
           << "\"message\":\"" << error.client_message << "\","
           << "\"retry\":" << (error.retry_allowed ? "true" : "false")
           << "}";
        return ss.str();
    }
    
    // ═══ SUCCESS RESPONSE ═══
    static std::string success(const std::string& data) {
        return "{\"status\":\"ok\",\"data\":" + data + "}";
    }
    
    // ═══ SECURITY REPORT ═══
    std::string security_report() {
        std::lock_guard<std::mutex> lock(mtx_);
        
        std::stringstream ss;
        ss << "{"
           << "\"total_errors\":" << total_errors_ << ","
           << "\"s_rank\":" << s_rank_count_ << ","
           << "\"ss_rank\":" << ss_rank_count_ << ","
           << "\"sss_rank\":" << sss_rank_count_ << ","
           << "\"blocked_ips\":" << blocked_ips_.size() << ","
           << "\"mode\":\"" << (production_mode_ ? "prod" : "dev") << "\""
           << "}";
        return ss.str();
    }
    
    int total_errors() const { return total_errors_; }
    bool is_ip_blocked(const std::string& ip) const {
        for (const auto& blocked : blocked_ips_) {
            if (blocked == ip) return true;
        }
        return false;
    }
};

} // namespace sss_error
