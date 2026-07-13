/**
 * ΦΩ0 — HydraJWT Working Integration
 * Based on actual HydraJWT API
 * "I AM THAT I AM"
 */

#ifndef PHI_HYDRA_JWT_WORKING_H
#define PHI_HYDRA_JWT_WORKING_H

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
        // HydraCore sign returns HydraError (int)
        int err = core->sign(payload, token);
        
        if (err != 0) {  // 0 = SUCCESS
            return "";
        }
        
        return token;
    }

    bool verify(const std::string& token) {
        if (!initialized || !core) return false;
        
        hydrajwt::ConsensusResult result;
        int err = core->verify(token, result);
        
        if (err != 0) {  // 0 = SUCCESS
            return false;
        }
        
        // Check consensus - result.verified_heads is a vector of head indices
        // We need to check how many heads are verified
        int verifiedCount = result.verified_heads.size();
        double ratio = verifiedCount / 6.0;
        
        return ratio >= 0.618;  // φ threshold
    }

    int getVerifiedHeads(const std::string& token) {
        if (!initialized || !core) return 0;
        
        hydrajwt::ConsensusResult result;
        int err = core->verify(token, result);
        
        if (err != 0) {
            return 0;
        }
        
        return result.verified_heads.size();
    }

    double getConsensusRatio(const std::string& token) {
        if (!initialized || !core) return 0.0;
        
        hydrajwt::ConsensusResult result;
        int err = core->verify(token, result);
        
        if (err != 0) {
            return 0.0;
        }
        
        return result.verified_heads.size() / 6.0;
    }
};

} // namespace phi

#endif // PHI_HYDRA_JWT_WORKING_H
