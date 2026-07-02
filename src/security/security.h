/*
 * FEmmg-FHE v22.2 — Unified Security Module
 *
 * Single include for all production security features:
 * - Φ-JWT Authentication
 * - Anti-Matter Rate Limiter
 * - Memory Guard (ARX network)
 * - Input Validator
 * - Session Manager
 * - Audit Logger
 * - Error Handler
 *
 * Production: Auto-enables all security features
 * Development: Security bypassed for testing
 */

#pragma once

#include "phi_jwt.h"
#include "anti_matter_v2.h"
#include "memory_guard.h"
#include "input_validator.h"
#include "session_manager.h"
#include "audit_log.h"
#include "error_handler.h"
#include <random>
#include <chrono>

namespace femmg_security {

// Generate a random session seed
inline uint64_t generate_session_seed() {
    std::random_device rd;
    uint64_t seed = 0;
    for (int i = 0; i < 4; i++) {
        uint64_t r = static_cast<uint64_t>(rd()) << 32 | static_cast<uint64_t>(rd());
        seed ^= r;
        seed = (seed << 13) | (seed >> 51);
    }
    auto now = std::chrono::high_resolution_clock::now().time_since_epoch().count();
    seed ^= static_cast<uint64_t>(now) ^ static_cast<uint64_t>(now >> 32);
    if (seed == 0) seed = 0x9E3779B97F4A7C15ULL;
    return seed;
}

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

    SecurityContext() {
        // Initialize with random session seed (NOT hardcoded!)
        mem_guard.init(generate_session_seed());
    }

    // Quick status
    std::string status() {
        return "{"
               "\"auth\":\"" + std::to_string(jwt.user_count()) + " users\","
               "\"rate_limiter\":\"active\","
               "\"memory_guard\":\"" + std::string(mem_guard.is_active() ? "active" : "inactive") + "\","
               "\"sessions\":\"" + std::to_string(sessions.active_sessions()) + " active\","
               "\"audit_events\":\"" + std::to_string(audit.count()) + "\""
               "}";
    }
};

} // namespace femmg_security
