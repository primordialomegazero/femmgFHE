#pragma once

#include "openfhe.h"
#include "../core/constants.h"

using namespace lbcrypto;

// ================================================================
// SECURE CONTEXT — FHE context wrapper
// ================================================================
struct SecureContext {
    CryptoContext<DCRTPoly> cc;
    KeyPair<DCRTPoly> kp;
};

// ================================================================
// CREATE FHE CONTEXT — Standard CKKS setup
// ================================================================
inline SecureContext create_fhe_context(
    uint32_t ringDim = DEFAULT_RING_DIM,
    uint32_t depth = DEFAULT_DEPTH,
    uint32_t batch = 0
) {
    uint32_t N_batch = (batch > 0) ? batch : (ringDim / 16);
    
    CCParams<CryptoContextCKKSRNS> p;
    p.SetRingDim(ringDim);
    p.SetMultiplicativeDepth(depth);
    p.SetScalingModSize(50);
    p.SetBatchSize(N_batch);
    p.SetSecretKeyDist(UNIFORM_TERNARY);
    p.SetSecurityLevel(HEStd_NotSet);
    
    auto cc = GenCryptoContext(p);
    cc->Enable(PKE);
    cc->Enable(KEYSWITCH);
    cc->Enable(LEVELEDSHE);
    cc->Enable(ADVANCEDSHE);
    
    auto kp = cc->KeyGen();
    cc->EvalMultKeyGen(kp.secretKey);
    cc->EvalSumKeyGen(kp.secretKey);
    
    return {cc, kp};
}
