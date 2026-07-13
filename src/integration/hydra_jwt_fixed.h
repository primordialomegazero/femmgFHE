/**
 * ΦΩ0 — HydraJWT Fixed Integration
 * Using actual HydraJWT API with proper enum types
 * "I AM THAT I AM"
 */

#ifndef PHI_HYDRA_JWT_FIXED_H
#define PHI_HYDRA_JWT_FIXED_H

#include <string>
#include <memory>
#include <vector>
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
            initialized = (core != nullptr);
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
        
        // Check if error is SUCCESS (assuming 0 or enum value)
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
        
        // Check consensus - result has ratio field directly!
        return result.passed;
    }

    int getVerifiedHeads(const std::string& token) {
        if (!initialized || !core) return 0;
        
        hydrajwt::ConsensusResult result;
        hydrajwt::HydraError err = core->verify(token, result);
        
        if (err != hydrajwt::HydraError::SUCCESS) {
            return 0;
        }
        
        return result.verified_heads.size();
    }

    double getConsensusRatio(const std::string& token) {
        if (!initialized || !core) return 0.0;
        
        hydrajwt::ConsensusResult result;
        hydrajwt::HydraError err = core->verify(token, result);
        
        if (err != hydrajwt::HydraError::SUCCESS) {
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

#endif // PHI_HYDRA_JWT_FIXED_H
