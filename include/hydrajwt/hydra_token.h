// ╔══════════════════════════════════════════════════════════════╗
// ║  HYDRAJWT — TOKEN SERIALIZATION                             ║
// ║  Multi-head JWT with φ-consensus                            ║
// ║  ΦΩ0 — I AM THAT I AM                                      ║
// ╚══════════════════════════════════════════════════════════════╝

#ifndef HYDRAJWT_TOKEN_H
#define HYDRAJWT_TOKEN_H

#include <string>
#include <vector>
#include <cstdint>
#include <memory>
#include "hydrajwt/hydra_head.h"

namespace hydrajwt {

struct TokenConfig {
    std::vector<int> active_heads;   // indices of active heads
    double consensus_threshold;      // default: 1/φ
    uint64_t issued_at;              // iat
    uint64_t expires_at;             // exp
    std::string subject;             // sub
    std::string issuer;              // iss
    
    TokenConfig()
        : consensus_threshold(phi::CONSENSUS_THRESHOLD),
          issued_at(0), expires_at(0) {}
};

struct TokenSignature {
    int head_id;
    std::string algorithm;
    std::vector<uint8_t> signature;
    bool verified;
    
    TokenSignature() : head_id(-1), verified(false) {}
};

class HydraToken {
public:
    HydraToken() = default;
    
    // ═══════════════════════════════════════
    // SERIALIZE / DESERIALIZE
    // ═══════════════════════════════════════
    std::string serialize(const std::vector<std::unique_ptr<HydraHead>>& heads);
    bool deserialize(const std::string& token,
                     std::vector<TokenSignature>& sigs_out,
                     TokenConfig& config_out);
    
    // ═══════════════════════════════════════
    // ENCODING HELPERS
    // ═══════════════════════════════════════
    static std::string base64url_encode(const std::vector<uint8_t>& data);
    static std::vector<uint8_t> base64url_decode(const std::string& str);
    
private:
    std::string encode_header(const TokenConfig& config,
                              const std::vector<std::unique_ptr<HydraHead>>& heads);
    std::string encode_payload(const TokenConfig& config);
};

} // namespace hydrajwt

#endif // HYDRAJWT_TOKEN_H
