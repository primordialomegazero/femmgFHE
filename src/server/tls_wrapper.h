/*
 * FEmmg-FHE v22.0.0 — TLS Wrapper
 * 
 * Minimal HTTPS support via OpenSSL.
 * Enable with: ./femmg_server --tls
 */

#pragma once
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <string>
#include <stdexcept>

class TLSServer {
private:
    SSL_CTX* ctx_;
    bool enabled_ = false;

public:
    TLSServer() = default;
    
    bool init(const std::string& cert_file, const std::string& key_file) {
        SSL_library_init();
        SSL_load_error_strings();
        OpenSSL_add_all_algorithms();
        
        ctx_ = SSL_CTX_new(TLS_server_method());
        if (!ctx_) return false;
        
        if (SSL_CTX_use_certificate_file(ctx_, cert_file.c_str(), SSL_FILETYPE_PEM) <= 0) return false;
        if (SSL_CTX_use_PrivateKey_file(ctx_, key_file.c_str(), SSL_FILETYPE_PEM) <= 0) return false;
        
        enabled_ = true;
        return true;
    }
    
    SSL* accept(int client_fd) {
        if (!enabled_) return nullptr;
        SSL* ssl = SSL_new(ctx_);
        SSL_set_fd(ssl, client_fd);
        if (SSL_accept(ssl) <= 0) {
            SSL_free(ssl);
            return nullptr;
        }
        return ssl;
    }
    
    int read(SSL* ssl, void* buf, int num) {
        return enabled_ ? SSL_read(ssl, buf, num) : recv(SSL_get_fd(ssl), buf, num, 0);
    }
    
    int write(SSL* ssl, const void* buf, int num) {
        return enabled_ ? SSL_write(ssl, buf, num) : send(SSL_get_fd(ssl), buf, num, 0);
    }
    
    void close(SSL* ssl) {
        if (ssl) {
            SSL_shutdown(ssl);
            SSL_free(ssl);
        }
    }
    
    bool is_enabled() const { return enabled_; }
    ~TLSServer() { if (ctx_) SSL_CTX_free(ctx_); }
};
