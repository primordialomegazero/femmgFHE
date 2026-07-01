/*
 * FEmmg-FHE v22.0.0 — Unified Security Module
 * 
 * Single include for all production security features:
 * - Φ-JWT Authentication
 * - Anti-Matter Rate Limiter
 * - Memory Guard
 * - Input Validator
 * - Session Manager
 * - Audit Logger
 * - Error Handler
 */

#pragma once

#include "phi_jwt.h"
#include "anti_matter_v2.h"
#include "memory_guard.h"
#include "input_validator.h"
#include "session_manager.h"
#include "audit_log.h"
#include "error_handler.h"

namespace femmg_security {

// ═══ UNIFIED SECURITY CONTEXT ═══
class SecurityContext {
public:
    phi_jwt::PhiJWT jwt;
    antimatter::AntiMatterV2 rate_limiter;
    memory_guard::MemoryGuard mem_guard;
    input_validator::InputValidator input_val;
    session_manager::SessionManager sessions;
    audit_log::AuditLogger audit;
    error_handler::ErrorHandler errors;
    
    SecurityContext(const std::string& jwt_secret = "femmg-fhe-v22") 
        : jwt(jwt_secret) {
        mem_guard.init(0x9E3779B97F4A7C15ULL);
    }
    
    // Quick status
    std::string status() {
        return "{"
               "\"auth\":\"" + std::to_string(jwt.user_count()) + " users\","
               "\"rate_limiter\":\"" + (rate_limiter.is_enabled() ? "active" : "disabled") + "\","
               "\"memory_guard\":\"" + (mem_guard.is_active() ? "active" : "inactive") + "\","
               "\"sessions\":\"" + std::to_string(sessions.active_sessions()) + " active\","
               "\"audit_events\":\"" + std::to_string(audit.count()) + "\""
               "}";
    }
};

} // namespace femmg_security
