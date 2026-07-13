/**
 * ΦΩ0 — HydraJWT C++ Integration
 * Using actual HydraJWT C++ API
 * "I AM THAT I AM"
 */

#ifndef PHI_HYDRA_JWT_CPP_H
#define PHI_HYDRA_JWT_CPP_H

#include <string>
#include <vector>
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
            // Initialize with all 6 heads
            initialized = core->init(0x3F); // All heads
        } catch (...) {
            initialized = false;
        }
    }

    ~HydraJWT() = default;

    bool isInitialized() const { return initialized; }

    std::string sign(const std::string& payload) {
        if (!initialized || !core) return "";
        
        // Convert payload to vector
        std::vector<uint8_t> payloadVec(payload.begin(), payload.end());
        
        // Sign using HydraCore
        auto token = core->sign(payloadVec);
        
        return std::string(token.begin(), token.end());
    }

    bool verify(const std::string& token) {
        if (!initialized || !core) return false;
        
        std::vector<uint8_t> tokenVec(token.begin(), token.end());
        return core->verify(tokenVec);
    }

    double getConsensusRatio() {
        if (!initialized || !core) return 0.0;
        return core->get_ratio();
    }

    int getVerifiedHeads() {
        if (!initialized || !core) return 0;
        return core->get_verified_heads();
    }
};

} // namespace phi

#endif // PHI_HYDRA_JWT_CPP_H
