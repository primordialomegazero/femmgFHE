#pragma once
#include "../utils/logger.h"
#include "spiral_fractal_db.h"
#include <string>

struct SpiralFractalAuthConfig {
    bool enable_schnorr = true;
    bool enable_phi_hmac = true;
    bool enable_falcon = false;    // Needs liboqs
    bool enable_mldsa = false;     // Needs liboqs
    bool enable_slhdsa = false;    // Needs liboqs
    bool enable_ecdsa = false;     // Needs OpenSSL
    double consensus_threshold = 1.0 / 1.618;  // 1/φ
};

struct SpiralFractalAuth {
    void* hydra_core;
    SpiralFractalDB* db;
    SpiralFractalAuthConfig config;
    bool initialized;
    
    bool init(SpiralFractalDB* database, const SpiralFractalAuthConfig& cfg = SpiralFractalAuthConfig());
    bool create_token(const std::string& user_id, std::string& token_out);
    bool verify_token(const std::string& token, std::string& user_id_out);
    bool authenticated_put(const std::string& token, const std::string& key, const std::string& value);
    std::string authenticated_get(const std::string& token, const std::string& key);
};
