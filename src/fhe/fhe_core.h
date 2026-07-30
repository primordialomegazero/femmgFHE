#pragma once
#include "../core/constants.h"
#include "../utils/logger.h"
#include "openfhe.h"
using namespace lbcrypto;

struct DualGate { Ciphertext<DCRTPoly> a; Ciphertext<DCRTPoly> b; };
struct SecureContext { CryptoContext<DCRTPoly> cc; KeyPair<DCRTPoly> kp; };

inline SecureContext create_fhe_context(uint32_t rd, uint32_t dp) {
    CCParams<CryptoContextCKKSRNS> p;
    p.SetMultiplicativeDepth(dp); p.SetScalingModSize(50); p.SetBatchSize(256);
    p.SetRingDim(rd); p.SetSecretKeyDist(UNIFORM_TERNARY); p.SetSecurityLevel(HEStd_NotSet);
    auto cc = GenCryptoContext(p);
    cc->Enable(PKE); cc->Enable(KEYSWITCH); cc->Enable(LEVELEDSHE);
    auto kp = cc->KeyGen(); cc->EvalMultKeyGen(kp.secretKey);
    Logger::info("FHE: RingDim="+std::to_string(rd)+" Depth="+std::to_string(dp));
    return {cc, kp};
}

inline DualGate enc(SecureContext& sc, double v) {
    return {sc.cc->Encrypt(sc.kp.publicKey, sc.cc->MakeCKKSPackedPlaintext(std::vector<double>{v})),
            sc.cc->Encrypt(sc.kp.publicKey, sc.cc->MakeCKKSPackedPlaintext(std::vector<double>{0.0}))};
}
inline double dec(SecureContext& sc, const Ciphertext<DCRTPoly>& c) {
    Plaintext pt; sc.cc->Decrypt(sc.kp.secretKey, c, &pt);
    return pt->GetCKKSPackedValue()[0].real();
}
inline double phi_val(DualGate& g, SecureContext& sc) { return dec(sc, g.a) + dec(sc, g.b) * PHI; }
inline double psi_val(DualGate& g, SecureContext& sc) { return dec(sc, g.a) + dec(sc, g.b) * PSI; }

inline DualGate nand_op(SecureContext& sc, DualGate& X, DualGate& Y) {
    auto a = sc.cc->EvalMult(X.a, Y.a);
    auto s = sc.cc->EvalAdd(sc.cc->EvalAdd(sc.cc->EvalMult(X.a, Y.b), sc.cc->EvalMult(X.b, Y.a)), sc.cc->EvalMult(X.b, Y.b));
    auto one = sc.cc->MakeCKKSPackedPlaintext(std::vector<double>{1.0});
    auto neg = sc.cc->MakeCKKSPackedPlaintext(std::vector<double>{-1.0});
    return {sc.cc->EvalSub(one, a), sc.cc->EvalMult(neg, s)};
}
inline DualGate and_op(SecureContext& sc, DualGate& X, DualGate& Y) { auto n = nand_op(sc, X, Y); return nand_op(sc, n, n); }
inline DualGate or_op(SecureContext& sc, DualGate& X, DualGate& Y) { auto nx = nand_op(sc, X, X), ny = nand_op(sc, Y, Y); return nand_op(sc, nx, ny); }
inline DualGate not_op(SecureContext& sc, DualGate& X) { return nand_op(sc, X, X); }
inline DualGate xor_op(SecureContext& sc, DualGate& X, DualGate& Y) {
    auto nx = not_op(sc, X), ny = not_op(sc, Y);
    auto x_and_ny = and_op(sc, X, ny), nx_and_y = and_op(sc, nx, Y);
    return or_op(sc, x_and_ny, nx_and_y);
}
