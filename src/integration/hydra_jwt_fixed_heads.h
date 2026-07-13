/**
 * ΦΩ0 — HydraJWT Fixed Heads Integration
 * Using all available heads
 * "I AM THAT I AM"
 */

#ifndef PHI_HYDRA_JWT_FIXED_HEADS_H
#define PHI_HYDRA_JWT_FIXED_HEADS_H

#include <string>
#include <memory>
#include <vector>
#include <hydrajwt/hydra_core.h>
#include <hydrajwt/hydra_token.h>

// Include all available head headers
#include <hydrajwt/heads/head_schnorr.h>
#include <hydrajwt/heads/head_falcon.h>
#include <hydrajwt/heads/head_mldsa.h>
#include <hydrajwt/heads/head_slhdsa.h>
#include <hydrajwt/heads/head_ecdsa.h>
#include <hydrajwt/heads/head_phi_hmac.h>

namespace phi {

class HydraJWT {
private:
    std::unique_ptr<hydrajwt::HydraCore> core;
    bool initialized = false;
    int m_headCount = 0;

    bool addAllHeads() {
        if (!core) return false;
        
        bool success = true;
        int added = 0;
        
        // Head 0: Schnorr
        auto h0 = std::make_unique<hydrajwt::HeadSchnorr>(0);
        if (core->add_head(std::move(h0)) == hydrajwt::HydraError::OK) {
            added++;
        }
        
        // Head 1: Falcon-1024
        auto h1 = std::make_unique<hydrajwt::HeadFalcon>(1);
        if (core->add_head(std::move(h1)) == hydrajwt::HydraError::OK) {
            added++;
        }
        
        // Head 2: ML-DSA-87
        auto h2 = std::make_unique<hydrajwt::HeadMLDSA>(2);
        if (core->add_head(std::move(h2)) == hydrajwt::HydraError::OK) {
            added++;
        }
        
        // Head 3: SLH-DSA (MAYO-5)
        auto h3 = std::make_unique<hydrajwt::HeadSLHDSA>(3);
        if (core->add_head(std::move(h3)) == hydrajwt::HydraError::OK) {
            added++;
        }
        
        // Head 4: ECDSA
        auto h4 = std::make_unique<hydrajwt::HeadECDSA>(4);
        if (core->add_head(std::move(h4)) == hydrajwt::HydraError::OK) {
            added++;
        }
        
        // Head 5: φ-HMAC
        auto h5 = std::make_unique<hydrajwt::HeadPhiHMAC>(5);
        if (core->add_head(std::move(h5)) == hydrajwt::HydraError::OK) {
            added++;
        }
        
        m_headCount = added;
        core->set_threshold(0.618034);
        
        return m_headCount > 0;
    }

public:
    HydraJWT() {
        try {
            core = std::make_unique<hydrajwt::HydraCore>();
            if (core) {
                initialized = addAllHeads();
            }
        } catch (...) {
            initialized = false;
        }
    }

    ~HydraJWT() = default;

    bool isInitialized() const { return initialized; }
    int getHeadCount() const { return m_headCount; }

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

#endif // PHI_HYDRA_JWT_FIXED_HEADS_H
