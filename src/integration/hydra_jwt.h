/**
 * ΦΩ0 — HydraJWT Integration
 * φ-Weighted Multi-Head JWT for FEmmg-FHE
 * "I AM THAT I AM"
 */

#ifndef PHI_HYDRA_JWT_H
#define PHI_HYDRA_JWT_H

#include <string>
#include <vector>
#include <cstring>
#include <cstdlib>

// Forward declare HydraCore
typedef struct HydraCore HydraCore;

// External C functions from HydraJWT
extern "C" {
    HydraCore* hydra_core_new();
    void hydra_core_free(HydraCore* core);
    int hydra_core_init(HydraCore* core, int flags);
    char* hydra_core_sign(HydraCore* core, const char* payload);
    int hydra_core_verify(HydraCore* core, const char* token);
    double hydra_core_get_ratio(HydraCore* core);
    int hydra_core_get_verified_heads(HydraCore* core);
}

#define HYDRA_ALL_HEADS 0x3F  // 6 heads: bits 0-5

namespace phi {

class HydraJWT {
private:
    HydraCore* core;
    bool initialized = false;

public:
    HydraJWT() : core(nullptr), initialized(false) {
        core = hydra_core_new();
        if (core) {
            initialized = (hydra_core_init(core, HYDRA_ALL_HEADS) == 0);
        }
    }

    ~HydraJWT() {
        if (core) {
            hydra_core_free(core);
            core = nullptr;
        }
    }

    bool isInitialized() const { return initialized; }

    std::string sign(const std::string& payload) {
        if (!initialized || !core) return "";
        char* token = hydra_core_sign(core, payload.c_str());
        if (!token) return "";
        std::string result(token);
        free(token);
        return result;
    }

    bool verify(const std::string& token) {
        if (!initialized || !core) return false;
        return hydra_core_verify(core, token.c_str()) == 1;
    }

    double getConsensusRatio() {
        if (!initialized || !core) return 0.0;
        return hydra_core_get_ratio(core);
    }

    int getVerifiedHeads() {
        if (!initialized || !core) return 0;
        return hydra_core_get_verified_heads(core);
    }
};

} // namespace phi

#endif // PHI_HYDRA_JWT_H
