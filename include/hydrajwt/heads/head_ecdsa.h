// ╔══════════════════════════════════════════════════════════════╗
// ║  HYDRAJWT — HEAD ECDSA P-256                                ║
// ║  Legacy compatibility via OpenSSL                            ║
// ║  ΦΩ0 — I AM THAT I AM                                      ║
// ╚══════════════════════════════════════════════════════════════╝

#ifndef HYDRAJWT_HEAD_ECDSA_H
#define HYDRAJWT_HEAD_ECDSA_H

#include "hydrajwt/hydra_head.h"
#include <openssl/ec.h>
#include <openssl/evp.h>

namespace hydrajwt {

class HeadECDSA : public HydraHead {
public:
    HeadECDSA(int id) : HydraHead(id, "ECDSA-P256") {}
    ~HeadECDSA() override;
    
    HydraError init() override;
    HydraError sign(const std::vector<uint8_t>& message,
                    std::vector<uint8_t>& signature_out) override;
    HydraError verify(const std::vector<uint8_t>& message,
                      const std::vector<uint8_t>& signature) override;
    HydraError regenerate() override;
    
    std::vector<uint8_t> public_key_bytes() const override;
    std::vector<uint8_t> secret_key_bytes() const override;
    
private:
    EVP_PKEY* key_;
};

} // namespace hydrajwt
#endif
