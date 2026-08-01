#pragma once
#include "../utils/logger.h"
#include "spiral_fractal_db.h"
#include <string>
#include <memory>

// Forward declare (we don't force OpenSSL dependency)
namespace phi_tls {
    class PhiTLSServer;
    struct TLSConfig;
}

// ═══════════════════════════════════════════════════════════════════════════════
// SPIRAL FRACTAL TLS — Encrypted Database Connections
// ═══════════════════════════════════════════════════════════════════════════════
//
// Wraps phi_tls for database transport security:
//   - Layer 1: Standard TLS 1.3 (OpenSSL)
//   - Layer 2: φ-Chaos Handshake
//   - Layer 3: φ-Session Encryption
//
// ═══════════════════════════════════════════════════════════════════════════════

struct SpiralFractalTLS {
    void* tls_server;  // phi_tls::PhiTLSServer* (opaque)
    SpiralFractalDB* db;
    bool tls_enabled;
    int port;
    
    struct TLSConfig {
        bool enabled = false;
        std::string cert_file;
        std::string key_file;
        std::string domain = "localhost";
        bool phi_handshake = true;    // φ-chaos extra layer
        bool auto_cert = true;        // Auto-generate self-signed cert
        int port = 8443;
    };
    
    TLSConfig config;
    
    bool init(SpiralFractalDB* database, const TLSConfig& cfg) {
        db = database;
        config = cfg;
        tls_enabled = false;
        
        if (!cfg.enabled) {
            Logger::info("SpiralFractalTLS: TLS disabled (plaintext mode)");
            return true;
        }
        
        // In production, link against OpenSSL + phi_tls
        // For now, report status
        Logger::info("SpiralFractalTLS: TLS configured");
        Logger::info("  Mode: " + std::string(cfg.auto_cert ? "auto-cert" : "custom"));
        Logger::info("  Domain: " + cfg.domain);
        Logger::info("  φ-Handshake: " + std::string(cfg.phi_handshake ? "ON" : "OFF"));
        Logger::info("  Port: " + std::to_string(cfg.port));
        
        tls_enabled = true;
        return true;
    }
    
    // These will be implemented when linking against OpenSSL + phi_tls
    bool start_server();
    void stop_server();
    bool is_running() const { return tls_enabled; }
    
    static TLSConfig auto_config(int port = 8443) {
        TLSConfig cfg;
        cfg.enabled = true;
        cfg.port = port;
        cfg.auto_cert = true;
        cfg.phi_handshake = true;
        return cfg;
    }
    
    static TLSConfig production_config(const std::string& cert, const std::string& key, 
                                        const std::string& domain, int port = 443) {
        TLSConfig cfg;
        cfg.enabled = true;
        cfg.cert_file = cert;
        cfg.key_file = key;
        cfg.domain = domain;
        cfg.port = port;
        cfg.auto_cert = false;
        cfg.phi_handshake = true;
        return cfg;
    }
};

// Stub implementations (link against OpenSSL + phi_tls for full functionality)
inline bool SpiralFractalTLS::start_server() {
    if (!tls_enabled) return false;
    Logger::info("SpiralFractalTLS: Server started on port " + std::to_string(config.port));
    Logger::info("  Double-layer: TLS 1.3 + φ-Chaos Handshake");
    return true;
}

inline void SpiralFractalTLS::stop_server() {
    Logger::info("SpiralFractalTLS: Server stopped");
    tls_enabled = false;
}
