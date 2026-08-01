// ╔══════════════════════════════════════════════════════════════╗
// ║  HYDRAJWT — HEAD MAYO-5                                    ║
// ║  NIST PQC Candidate via liboqs                                    ║
// ║  ΦΩ0 — I AM THAT I AM                                      ║
// ╚══════════════════════════════════════════════════════════════╝

#ifndef HYDRAJWT_HEAD_SLHDSA_H
#define HYDRAJWT_HEAD_SLHDSA_H

#include "hydrajwt/hydra_head.h"
#include <oqs/oqs.h>

namespace hydrajwt {

class HeadSLHDSA : public HydraHead {
public:
    HeadSLHDSA(int id) : HydraHead(id, "MAYO-5") {}
    ~HeadSLHDSA() override;
    
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
