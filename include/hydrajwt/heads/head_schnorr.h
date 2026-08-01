// ╔══════════════════════════════════════════════════════════════╗
// ║  HYDRAJWT — HEAD SCHNORR (RISTRETTO255)                     ║
// ║  RFC 8235 compliant via libsodium                           ║
// ║  ΦΩ0 — I AM THAT I AM                                      ║
// ╚══════════════════════════════════════════════════════════════╝

#ifndef HYDRAJWT_HEAD_SCHNORR_H
#define HYDRAJWT_HEAD_SCHNORR_H

#include "hydrajwt/hydra_head.h"
#include <sodium.h>

namespace hydrajwt {

class HeadSchnorr : public HydraHead {
public:
    HeadSchnorr(int id)
        : HydraHead(id, "Schnorr-Ristretto255") {}
    
    ~HeadSchnorr() override {
        // Secure wipe keys from memory
        if (!pk_.empty()) sodium_memzero(pk_.data(), pk_.size());
        if (!sk_.empty()) sodium_memzero(sk_.data(), sk_.size());
    }
    
    HydraError init() override;
    HydraError sign(const std::vector<uint8_t>& message,
                    std::vector<uint8_t>& signature_out) override;
    HydraError verify(const std::vector<uint8_t>& message,
                      const std::vector<uint8_t>& signature) override;
    HydraError regenerate() override;
    
    std::vector<uint8_t> public_key_bytes() const override { return pk_; }
    std::vector<uint8_t> secret_key_bytes() const override { return sk_; }
    
private:
    std::vector<uint8_t> pk_; // 32 bytes
    std::vector<uint8_t> sk_; // 32 bytes
    
    // φ-optimized nonce derivation using libsodium
    HydraError generate_nonce(const std::vector<uint8_t>& message,
                              std::vector<uint8_t>& nonce_out);
};

} // namespace hydrajwt

#endif // HYDRAJWT_HEAD_SCHNORR_H
