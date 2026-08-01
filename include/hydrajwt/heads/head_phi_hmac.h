// ╔══════════════════════════════════════════════════════════════╗
// ║  HYDRAJWT — HEAD φ-HMAC                                     ║
// ║  Custom φ-based symmetric signing                            ║
// ║  ΦΩ0 — I AM THAT I AM                                      ║
// ╚══════════════════════════════════════════════════════════════╝

#ifndef HYDRAJWT_HEAD_PHI_HMAC_H
#define HYDRAJWT_HEAD_PHI_HMAC_H

#include "hydrajwt/hydra_head.h"
#include "hydrajwt/phi_utils.h"

namespace hydrajwt {

class HeadPhiHMAC : public HydraHead {
public:
    HeadPhiHMAC(int id) : HydraHead(id, "φ-HMAC-256") {}
    ~HeadPhiHMAC() override;
    
    HydraError init() override;
    HydraError sign(const std::vector<uint8_t>& message,
                    std::vector<uint8_t>& signature_out) override;
    HydraError verify(const std::vector<uint8_t>& message,
                      const std::vector<uint8_t>& signature) override;
    HydraError regenerate() override;
    
    std::vector<uint8_t> public_key_bytes() const override { return key_; }
    std::vector<uint8_t> secret_key_bytes() const override { return key_; }
    
private:
    std::vector<uint8_t> key_;
    HydraError phi_hmac(const std::vector<uint8_t>& message,
                        std::vector<uint8_t>& mac_out);
};

} // namespace hydrajwt
#endif
