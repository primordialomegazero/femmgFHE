/**
 * ΦΩ0 — HydraJWT Complete Integration
 * With proper head management
 * "I AM THAT I AM"
 */

#ifndef PHI_HYDRA_JWT_COMPLETE_H
#define PHI_HYDRA_JWT_COMPLETE_H

#include <string>
#include <memory>
#include <vector>
#include <hydrajwt/hydra_core.h>
#include <hydrajwt/hydra_token.h>
#include <hydrajwt/heads/head_schnorr.h>
#include <hydrajwt/heads/head_falcon.h>
#include <hydrajwt/heads/head_mldsa.h>
#include <hydrajwt/heads/head_ecdsa.h>
#include <hydrajwt/heads/head_phi_hmac.h>

namespace phi {

class HydraJWT {
private:
    std::unique_ptr<hydrajwt::HydraCore> core;
    bool initialized = false;

    bool addAllHeads() {
        if (!core) return false;
        
        bool allAdded = true;
        
        // Head 0: Schnorr-Ristretto255 (Classical)
        auto head0 = std::make_unique<hydrajwt::HeadSchnorr>(0);
        allAdded &= (core->add_head(std::move(head0)) == hydrajwt::HydraError::OK);
        
        // Head 1: Falcon-1024 (NIST Level 5)
        auto head1 = std::make_unique<hydrajwt::HeadFalcon>(1);
        allAdded &= (core->add_head(std::move(head1)) == hydrajwt::HydraError::OK);
        
        // Head 2: ML-DSA-87 (NIST FIPS 204)
        auto head2 = std::make_unique<hydrajwt::HeadMLDSA>(2);
        allAdded &= (core->add_head(std::move(head2)) == hydrajwt::HydraError::OK);
        
        // Head 4: ECDSA P-256 (Legacy)
        auto head4 = std::make_unique<hydrajwt::HeadECDSA>(4);
        allAdded &= (core->add_head(std::move(head4)) == hydrajwt::HydraError::OK);
        
        // Head 5: φ-HMAC-256 (Custom)
        auto head5 = std::make_unique<hydrajwt::HeadPhiHMAC>(5);
        allAdded &= (core->add_head(std::move(head5)) == hydrajwt::HydraError::OK);
        
        // Set threshold to φ
        core->set_threshold(0.618034);
        
        return allAdded && (core->head_count() > 0);
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
    size_t headCount() const { return core ? core->head_count() : 0; }

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

#endif // PHI_HYDRA_JWT_COMPLETE_H
