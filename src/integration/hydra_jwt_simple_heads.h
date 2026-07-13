/**
 * ΦΩ0 — HydraJWT Simple Heads Integration
 * Using only available heads
 * "I AM THAT I AM"
 */

#ifndef PHI_HYDRA_JWT_SIMPLE_HEADS_H
#define PHI_HYDRA_JWT_SIMPLE_HEADS_H

#include <string>
#include <memory>
#include <vector>
#include <hydrajwt/hydra_core.h>
#include <hydrajwt/hydra_token.h>

// Include available head headers
#include <hydrajwt/heads/head_schnorr.h>
#include <hydrajwt/heads/head_ecdsa.h>
#include <hydrajwt/heads/head_phi_hmac.h>

// These may or may not be available
#ifdef HYDRAJWT_HEAD_FALCON_H
#include <hydrajwt/heads/head_falcon.h>
#endif

#ifdef HYDRAJWT_HEAD_MLDSA_H
#include <hydrajwt/heads/head_mldsa.h>
#endif

namespace phi {

class HydraJWT {
private:
    std::unique_ptr<hydrajwt::HydraCore> core;
    bool initialized = false;
    int headCount = 0;

    bool addHeads() {
        if (!core) return false;
        
        // Head 0: Schnorr (always available)
        auto h0 = std::make_unique<hydrajwt::HeadSchnorr>(0);
        if (core->add_head(std::move(h0)) == hydrajwt::HydraError::OK) {
            headCount++;
        }
        
        // Head 4: ECDSA (always available)
        auto h4 = std::make_unique<hydrajwt::HeadECDSA>(4);
        if (core->add_head(std::move(h4)) == hydrajwt::HydraError::OK) {
            headCount++;
        }
        
        // Head 5: φ-HMAC (always available)
        auto h5 = std::make_unique<hydrajwt::HeadPhiHMAC>(5);
        if (core->add_head(std::move(h5)) == hydrajwt::HydraError::OK) {
            headCount++;
        }
        
        // Set threshold to φ
        core->set_threshold(0.618034);
        
        return headCount > 0;
    }

public:
    HydraJWT() {
        try {
            core = std::make_unique<hydrajwt::HydraCore>();
            if (core) {
                initialized = addHeads();
            }
        } catch (...) {
            initialized = false;
        }
    }

    ~HydraJWT() = default;

    bool isInitialized() const { return initialized; }
    int headCount() const { return headCount; }

    std::string sign(const std::string& payload) {
        if (!initialized || !core) return "";
        
        std::string token;
        auto err = core->sign(payload, token);
        
        if (err != hydrajwt::HydraError::OK) {
            return "";
        }
        
        return token;
    }

    bool verify(const std::string& token) {
        if (!initialized || !core) return false;
        
        hydrajwt::ConsensusResult result;
        auto err = core->verify(token, result);
        
        if (err != hydrajwt::HydraError::OK) {
            return false;
        }
        
        return result.passed;
    }

    int getVerifiedHeads(const std::string& token) {
        if (!initialized || !core) return 0;
        
        hydrajwt::ConsensusResult result;
        auto err = core->verify(token, result);
        
        if (err != hydrajwt::HydraError::OK) {
            return 0;
        }
        
        return static_cast<int>(result.verified_heads.size());
    }

    double getConsensusRatio(const std::string& token) {
        if (!initialized || !core) return 0.0;
        
        hydrajwt::ConsensusResult result;
        auto err = core->verify(token, result);
        
        if (err != hydrajwt::HydraError::OK) {
            return 0.0;
        }
        
        return result.ratio;
    }

    double getThreshold() {
        if (!initialized || !core) return 0.0;
        return core->threshold();
    }
};

} // namespace phi

#endif // PHI_HYDRA_JWT_SIMPLE_HEADS_H
