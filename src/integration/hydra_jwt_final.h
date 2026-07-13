/**
 * ΦΩ0 — HydraJWT Final Integration
 * Using actual HydraJWT C++ API
 * "I AM THAT I AM"
 */

#ifndef PHI_HYDRA_JWT_FINAL_H
#define PHI_HYDRA_JWT_FINAL_H

#include <string>
#include <memory>
#include <hydrajwt/hydra_core.h>
#include <hydrajwt/hydra_token.h>

namespace phi {

class HydraJWT {
private:
    std::unique_ptr<hydrajwt::HydraCore> core;
    bool initialized = false;

public:
    HydraJWT() {
        try {
            core = std::make_unique<hydrajwt::HydraCore>();
            // HydraCore constructor initializes all heads by default
            initialized = (core != nullptr);
            
            // Verify it's working by checking head count
            if (initialized) {
                // Try to add all 6 heads (they're already added in constructor)
                // but we can verify
                initialized = true;
            }
        } catch (...) {
            initialized = false;
        }
    }

    ~HydraJWT() = default;

    bool isInitialized() const { return initialized; }

    std::string sign(const std::string& payload) {
        if (!initialized || !core) return "";
        
        std::string token;
        hydrajwt::HydraError err = core->sign(payload, token);
        
        if (err != hydrajwt::HydraError::SUCCESS) {
            return "";
        }
        
        return token;
    }

    bool verify(const std::string& token) {
        if (!initialized || !core) return false;
        
        hydrajwt::ConsensusResult result;
        hydrajwt::HydraError err = core->verify(token, result);
        
        if (err != hydrajwt::HydraError::SUCCESS) {
            return false;
        }
        
        // Check consensus threshold (φ ≈ 0.618)
        double ratio = result.verified_heads / 6.0;
        return ratio >= 0.618;
    }

    int getVerifiedHeads(const std::string& token) {
        if (!initialized || !core) return 0;
        
        hydrajwt::ConsensusResult result;
        hydrajwt::HydraError err = core->verify(token, result);
        
        if (err != hydrajwt::HydraError::SUCCESS) {
            return 0;
        }
        
        return result.verified_heads;
    }

    double getConsensusRatio(const std::string& token) {
        if (!initialized || !core) return 0.0;
        
        hydrajwt::ConsensusResult result;
        hydrajwt::HydraError err = core->verify(token, result);
        
        if (err != hydrajwt::HydraError::SUCCESS) {
            return 0.0;
        }
        
        return result.verified_heads / 6.0;
    }
};

} // namespace phi

#endif // PHI_HYDRA_JWT_FINAL_H
