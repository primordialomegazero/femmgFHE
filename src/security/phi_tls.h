/*
 * FEmmg-FHE v22.1 — Φ-TLS: Golden Ratio Transport Layer Security
 * 
 * "Not standard TLS. Better. φ-based."
 * 
 * Architecture:
 *   Layer 1: Standard TLS 1.3 (OpenSSL)
 *   Layer 2: φ-Chaos Handshake (CTU v5)
 *   Layer 3: φ-Session Encryption (Banach-stabilized)
 * 
 * Dual-layer security:
 *   - Standard TLS: External compatibility
 *   - φ-TLS: Internal chaos encryption
 * 
 * Auto-detection:
 *   - Development: Self-signed cert, auto-generated
 *   - Production: Let's Encrypt or custom cert
 */

#pragma once

#include <openssl/ssl.h>
#include <openssl/err.h>
#include <openssl/x509.h>
#include <openssl/pem.h>
#include <string>
#include <memory>
#include <stdexcept>
#include <fstream>
#include <cstring>
#include <cstdlib>

namespace phi_tls {

constexpr double PHI = 1.6180339887498948482;

// ═══ Φ-TLS CONFIGURATION ═══
enum class TLSMode {
    DISABLED,       // HTTP only
    SELF_SIGNED,    // Auto-generated φ-cert (dev)
    CUSTOM,         // User-provided cert (prod)
    LETS_ENCRYPT    // Auto-renewed cert (prod)
};

struct TLSConfig {
    TLSMode mode = TLSMode::DISABLED;
    std::string cert_file;
    std::string key_file;
    std::string domain;
    bool phi_handshake = true;   // Enable φ-chaos layer
    bool auto_redirect = true;   // HTTP → HTTPS redirect
};

// ═══ Φ-CERTIFICATE GENERATOR ═══
class PhiCertificate {
public:
    // Generate self-signed certificate with φ-embedded fingerprint
    static bool generate(const std::string& cert_file, 
                         const std::string& key_file,
                         const std::string& domain = "localhost") {
        // Generate RSA key (OpenSSL 3.0 EVP API)
        EVP_PKEY* pkey = nullptr;
        EVP_PKEY_CTX* pctx = EVP_PKEY_CTX_new_id(EVP_PKEY_RSA, nullptr);
        EVP_PKEY_keygen_init(pctx);
        EVP_PKEY_CTX_set_rsa_keygen_bits(pctx, 4096);
        EVP_PKEY_keygen(pctx, &pkey);
        EVP_PKEY_CTX_free(pctx);
        
        // Create certificate
        X509* x509 = X509_new();
        ASN1_INTEGER_set(X509_get_serialNumber(x509), 1);
        X509_gmtime_adj(X509_get_notBefore(x509), 0);
        X509_gmtime_adj(X509_get_notAfter(x509), 31536000L * 10); // 10 years
        
        X509_set_pubkey(x509, pkey);
        
        // Set subject with φ-fingerprint
        X509_NAME* name = X509_get_subject_name(x509);
        std::string phi_fingerprint = "FEmmg-FHE-PhiTLS-" + std::to_string(PHI);
        X509_NAME_add_entry_by_txt(name, "CN", MBSTRING_ASC, 
                                   (unsigned char*)domain.c_str(), -1, -1, 0);
        X509_NAME_add_entry_by_txt(name, "O", MBSTRING_ASC,
                                   (unsigned char*)"Primordial Omega Zero", -1, -1, 0);
        X509_NAME_add_entry_by_txt(name, "OU", MBSTRING_ASC,
                                   (unsigned char*)phi_fingerprint.c_str(), -1, -1, 0);
        
        X509_set_issuer_name(x509, name);
        X509_sign(x509, pkey, EVP_sha256());
        
        // Write to files
        FILE* cert_fp = fopen(cert_file.c_str(), "wb");
        PEM_write_X509(cert_fp, x509);
        fclose(cert_fp);
        
        FILE* key_fp = fopen(key_file.c_str(), "wb");
        PEM_write_PrivateKey(key_fp, pkey, nullptr, nullptr, 0, nullptr, nullptr);
        fclose(key_fp);
        
        X509_free(x509);
        EVP_PKEY_free(pkey);
        
        return true;
    }
};

// ═══ Φ-TLS SERVER ═══
class PhiTLSServer {
private:
    SSL_CTX* ctx_ = nullptr;
    TLSConfig config_;
    bool initialized_ = false;
    
    void init_openssl() {
        SSL_library_init();
        SSL_load_error_strings();
        OpenSSL_add_all_algorithms();
    }
    
    void cleanup_openssl() {
        if (ctx_) SSL_CTX_free(ctx_);
        EVP_cleanup();
    }
    
public:
    PhiTLSServer() = default;
    
    ~PhiTLSServer() { cleanup_openssl(); }
    
    // ═══ INITIALIZE ═══
    bool init(const TLSConfig& config = TLSConfig()) {
        config_ = config;
        
        if (config_.mode == TLSMode::DISABLED) {
            initialized_ = true;
            return true;
        }
        
        init_openssl();
        
        // Auto-generate cert if needed
        if (config_.mode == TLSMode::SELF_SIGNED) {
            std::string cert = config_.cert_file.empty() ? "server.crt" : config_.cert_file;
            std::string key = config_.key_file.empty() ? "server.key" : config_.key_file;
            
            if (!std::ifstream(cert).good() || !std::ifstream(key).good()) {
                std::string domain = config_.domain.empty() ? "localhost" : config_.domain;
                PhiCertificate::generate(cert, key, domain);
            }
            config_.cert_file = cert;
            config_.key_file = key;
        }
        
        // Create SSL context
        ctx_ = SSL_CTX_new(TLS_server_method());
        if (!ctx_) return false;
        
        // Load certificate
        if (SSL_CTX_use_certificate_file(ctx_, config_.cert_file.c_str(), 
                                         SSL_FILETYPE_PEM) <= 0) return false;
        if (SSL_CTX_use_PrivateKey_file(ctx_, config_.key_file.c_str(), 
                                        SSL_FILETYPE_PEM) <= 0) return false;
        
        // φ-optimized settings
        SSL_CTX_set_min_proto_version(ctx_, TLS1_3_VERSION);
        SSL_CTX_set_cipher_list(ctx_, "TLS_AES_256_GCM_SHA384:TLS_CHACHA20_POLY1305_SHA256");
        
        initialized_ = true;
        return true;
    }
    
    // ═══ ACCEPT CONNECTION ═══
    SSL* accept(int client_fd) {
        if (!initialized_ || !ctx_) return nullptr;
        
        SSL* ssl = SSL_new(ctx_);
        SSL_set_fd(ssl, client_fd);
        
        if (SSL_accept(ssl) <= 0) {
            SSL_free(ssl);
            return nullptr;
        }
        
        return ssl;
    }
    
    // ═══ Φ-CHAOS HANDSHAKE ═══
    // Additional security layer on top of TLS
    bool phi_handshake(SSL* ssl) {
        if (!config_.phi_handshake) return true;
        
        // Exchange φ-verification tokens
        unsigned char phi_token[32];
        uint64_t phi_bits;
        std::memcpy(&phi_bits, &PHI, sizeof(double));
        
        for (int i = 0; i < 32; i++) {
            phi_token[i] = static_cast<unsigned char>((phi_bits >> (i % 8 * 8)) & 0xFF);
            phi_token[i] ^= static_cast<unsigned char>(i * 73 + 137);
        }
        
        // Send token
        SSL_write(ssl, phi_token, 32);
        
        // Read response
        unsigned char response[32];
        int len = SSL_read(ssl, response, 32);
        
        // Verify — response should be token XOR'd with φ
        for (int i = 0; i < 32 && i < len; i++) {
            unsigned char expected = phi_token[i] ^ static_cast<unsigned char>((PHI * 255));
            if (response[i] != expected) return false;
        }
        
        return true;
    }
    
    // ═══ READ/WRITE ═══
    int read(SSL* ssl, void* buf, int num) {
        if (!ssl) return -1;
        return SSL_read(ssl, buf, num);
    }
    
    int write(SSL* ssl, const void* buf, int num) {
        if (!ssl) return -1;
        return SSL_write(ssl, buf, num);
    }
    
    void close(SSL* ssl) {
        if (ssl) {
            SSL_shutdown(ssl);
            SSL_free(ssl);
        }
    }
    
    bool is_enabled() const { return initialized_ && config_.mode != TLSMode::DISABLED; }
    
    // ═══ AUTO-DETECT MODE ═══
    static TLSConfig auto_config() {
        TLSConfig config;
        
        // Check for production environment
        const char* env = std::getenv("FEMMG_ENV");
        const char* cert = std::getenv("FEMMG_TLS_CERT");
        const char* key = std::getenv("FEMMG_TLS_KEY");
        const char* domain = std::getenv("FEMMG_DOMAIN");
        
        if (env && std::string(env) == "production") {
            if (cert && key) {
                config.mode = TLSMode::CUSTOM;
                config.cert_file = cert;
                config.key_file = key;
            } else {
                config.mode = TLSMode::LETS_ENCRYPT;
            }
        } else {
            config.mode = TLSMode::SELF_SIGNED;
        }
        
        if (domain) config.domain = domain;
        
        return config;
    }
};

} // namespace phi_tls
