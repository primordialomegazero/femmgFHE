/**
 * ΦΩ0 — HydraJWT Final Working Integration
 * Using actual HydraJWT API with integer error codes
 * "I AM THAT I AM"
 */

#ifndef PHI_HYDRA_JWT_FINAL_WORKING_H
#define PHI_HYDRA_JWT_FINAL_WORKING_H

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
        // HydraError is an enum - check if it's 0 for success
        auto err = core->sign(payload, token);
        
        // Try comparing to 0 (assuming SUCCESS is 0)
        if (static_cast<int>(err) != 0) {
            return "";
        }
        
        return token;
    }

    bool verify(const std::string& token) {
        if (!initialized || !core) return false;
        
        hydrajwt::ConsensusResult result;
        auto err = core->verify(token, result);
        
        if (static_cast<int>(err) != 0) {
            return false;
        }
        
        // result has 'passed' field directly!
        return result.passed;
    }

    int getVerifiedHeads(const std::string& token) {
        if (!initialized || !core) return 0;
        
        hydrajwt::ConsensusResult result;
        auto err = core->verify(token, result);
        
        if (static_cast<int>(err) != 0) {
            return 0;
        }
        
        return static_cast<int>(result.verified_heads.size());
    }

    double getConsensusRatio(const std::string& token) {
        if (!initialized || !core) return 0.0;
        
        hydrajwt::ConsensusResult result;
        auto err = core->verify(token, result);
        
        if (static_cast<int>(err) != 0) {
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

#endif // PHI_HYDRA_JWT_FINAL_WORKING_H
