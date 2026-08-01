// ╔══════════════════════════════════════════════════════════════╗
// ║  HYDRAJWT — HEAD FALCON-1024                                ║
// ║  NIST FIPS 204 Level 5 via liboqs                            ║
// ║  ΦΩ0 — I AM THAT I AM                                      ║
// ╚══════════════════════════════════════════════════════════════╝

#ifndef HYDRAJWT_HEAD_FALCON_H
#define HYDRAJWT_HEAD_FALCON_H

#include "hydrajwt/hydra_head.h"
#include <oqs/oqs.h>

namespace hydrajwt {

class HeadFalcon : public HydraHead {
public:
    HeadFalcon(int id) : HydraHead(id, "Falcon-1024") {}
    ~HeadFalcon() override;
    
    HydraError init() override;
    HydraError sign(const std::vector<uint8_t>& message,
                    std::vector<uint8_t>& signature_out) override;
    HydraError verify(const std::vector<uint8_t>& message,
                      const std::vector<uint8_t>& signature) override;
    HydraError regenerate() override;
    
    std::vector<uint8_t> public_key_bytes() const override { return pk_; }
    std::vector<uint8_t> secret_key_bytes() const override { return sk_; }
    
private:
    std::vector<uint8_t> pk_;
    std::vector<uint8_t> sk_;
    OQS_SIG* sig_;
};

} // namespace hydrajwt
#endif
