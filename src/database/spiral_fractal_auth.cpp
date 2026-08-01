#include "spiral_fractal_auth.h"
#include <hydrajwt/hydra_core.h>
#include <hydrajwt/heads/head_schnorr.h>
#include <hydrajwt/heads/head_phi_hmac.h>

// Optional heads — uncomment when liboqs/OpenSSL are available
// #include <hydrajwt/heads/head_falcon.h>
// #include <hydrajwt/heads/head_mldsa.h>
// #include <hydrajwt/heads/head_slhdsa.h>
// #include <hydrajwt/heads/head_ecdsa.h>

bool SpiralFractalAuth::init(SpiralFractalDB* database, const SpiralFractalAuthConfig& cfg) {
    db = database;
    config = cfg;
    
    auto* core = new hydrajwt::HydraCore();
    int heads_added = 0;
    
    if (config.enable_schnorr) {
        auto head = std::make_unique<hydrajwt::HeadSchnorr>(heads_added);
        if (head->init() == hydrajwt::HydraError::OK) {
            core->add_head(std::move(head));
            heads_added++;
        }
    }
    
    if (config.enable_phi_hmac) {
        auto head = std::make_unique<hydrajwt::HeadPhiHMAC>(heads_added);
        if (head->init() == hydrajwt::HydraError::OK) {
            core->add_head(std::move(head));
            heads_added++;
        }
    }
    
    // More heads here when libraries are available
    
    if (heads_added == 0) {
        delete core;
        Logger::error("Auth: no heads could be initialized");
        return false;
    }
    
    core->set_threshold(config.consensus_threshold);
    hydra_core = core;
    initialized = true;
    
    Logger::info("SpiralFractalAuth: " + std::to_string(heads_added) + 
                " heads active (Schnorr + φ-HMAC)");
    return true;
}

bool SpiralFractalAuth::create_token(const std::string& user_id, std::string& token_out) {
    if (!initialized) return false;
    auto* core = static_cast<hydrajwt::HydraCore*>(hydra_core);
    
    std::string payload = "{\"user\":\"" + user_id + "\",\"iat\":" + std::to_string(time(0)) + "}";
    return core->sign(payload, token_out) == hydrajwt::HydraError::OK;
}

bool SpiralFractalAuth::verify_token(const std::string& token, std::string& user_id_out) {
    if (!initialized) return false;
    auto* core = static_cast<hydrajwt::HydraCore*>(hydra_core);
    
    hydrajwt::ConsensusResult result;
    if (core->verify(token, result) != hydrajwt::HydraError::OK || !result.passed) {
        return false;
    }
    
    user_id_out = "authenticated";
    return true;
}

bool SpiralFractalAuth::authenticated_put(const std::string& token, const std::string& key, const std::string& value) {
    std::string uid;
    if (!verify_token(token, uid)) return false;
    return db->put(key, value);
}

std::string SpiralFractalAuth::authenticated_get(const std::string& token, const std::string& key) {
    std::string uid;
    if (!verify_token(token, uid)) return "";
    return db->get(key);
}
