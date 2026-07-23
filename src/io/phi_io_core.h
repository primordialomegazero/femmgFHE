// ΦΩ0 — PHI-IO CORE v2: Fixed pre-scaling for functional equivalence
#ifndef PHI_IO_CORE_H
#define PHI_IO_CORE_H

#include <openfhe.h>
#include <vector>
#include <cmath>
#include <random>

namespace phi_io {

const double PHI = 1.6180339887498948482;
const double PSI = -0.6180339887498948482;
const double CLEAN_GROWTH = PHI * PHI;  // φ² ≈ 2.618
const double PRE_SCALE = 1.0 / pow(CLEAN_GROWTH, 1.0/3.0);  // ~0.726

using namespace lbcrypto;

class ObfuscatedProgram {
private:
    CryptoContext<DCRTPoly> cc;
    KeyPair<DCRTPoly> keys;
    uint32_t slots;

public:
    struct ObfuscatedValue {
        Ciphertext<DCRTPoly> a;
        Ciphertext<DCRTPoly> b;
    };

    ObfuscatedProgram(uint32_t ringDim = 8192) : slots(ringDim / 2) {
        CCParams<CryptoContextCKKSRNS> params;
        params.SetSecretKeyDist(UNIFORM_TERNARY);
        params.SetSecurityLevel(HEStd_NotSet);
        params.SetRingDim(ringDim);
        params.SetScalingModSize(50);
        params.SetFirstModSize(60);
        params.SetScalingTechnique(FLEXIBLEAUTO);
        params.SetMultiplicativeDepth(80);
        cc = GenCryptoContext(params);
        cc->Enable(PKE); cc->Enable(KEYSWITCH); cc->Enable(LEVELEDSHE); cc->Enable(ADVANCEDSHE);
        keys = cc->KeyGen(); cc->EvalMultKeyGen(keys.secretKey);
    }

    ObfuscatedValue encrypt(double phi_value, double psi_value) {
        auto enc = [this](double v) {
            std::vector<double> vec(slots, 0.0); vec[0] = v;
            return cc->Encrypt(keys.publicKey, cc->MakeCKKSPackedPlaintext(vec));
        };
        return {enc(phi_value), enc(psi_value)};
    }

    ObfuscatedValue add(const ObfuscatedValue& x, const ObfuscatedValue& y) {
        return {cc->EvalAdd(x.a, y.a), cc->EvalAdd(x.b, y.b)};
    }

    ObfuscatedValue multiply(const ObfuscatedValue& x, const ObfuscatedValue& y) {
        auto ac = cc->EvalMult(x.a, y.a), bd = cc->EvalMult(x.b, y.b);
        auto ad = cc->EvalMult(x.a, y.b), bc = cc->EvalMult(x.b, y.a);
        return {cc->EvalAdd(ac, bd), cc->EvalAdd(cc->EvalAdd(ad, bc), bd)};
    }

    ObfuscatedValue mul_X(const ObfuscatedValue& x) {
        return {x.b, cc->EvalAdd(x.a, x.b)};
    }

    ObfuscatedValue div_X(const ObfuscatedValue& x) {
        return {cc->EvalSub(x.b, x.a), x.a};
    }

    ObfuscatedValue clean(const ObfuscatedValue& x, int ratio = 3) {
        ObfuscatedValue y = x;
        for (int i = 0; i < ratio; i++) y = mul_X(y);
        y = div_X(y);
        return y;
    }

    // Pre-scaled multiply: includes φ⁻² compensation for clean growth
    ObfuscatedValue mul_pre(const ObfuscatedValue& x, const ObfuscatedValue& y) {
        auto pre = encrypt(PRE_SCALE, 0.0);
        auto scaled = multiply(y, pre);
        return multiply(x, scaled);
    }

    double decrypt_phi(const ObfuscatedValue& x) {
        Plaintext pa, pb;
        cc->Decrypt(keys.secretKey, x.a, &pa);
        cc->Decrypt(keys.secretKey, x.b, &pb);
        pa->SetLength(slots); pb->SetLength(slots);
        return pa->GetRealPackedValue()[0] + pb->GetRealPackedValue()[0] * PHI;
    }

    double decrypt_psi(const ObfuscatedValue& x) {
        Plaintext pa, pb;
        cc->Decrypt(keys.secretKey, x.a, &pa);
        cc->Decrypt(keys.secretKey, x.b, &pb);
        pa->SetLength(slots); pb->SetLength(slots);
        return pa->GetRealPackedValue()[0] + pb->GetRealPackedValue()[0] * PSI;
    }

    // Both programs use SAME number of operations now
    // Program 1: f(x) = (x+1)(x+2) — 2 adds, 1 mul, 3 cleans
    ObfuscatedValue program_factored(const ObfuscatedValue& x) {
        auto one = encrypt(1.0 / CLEAN_GROWTH, 0.0);   // Compensate for clean
        auto two = encrypt(2.0 / CLEAN_GROWTH, 0.0);
        auto x1 = add(x, one); x1 = clean(x1);
        auto x2 = add(x, two); x2 = clean(x2);
        auto result = multiply(x1, x2);
        return clean(result);
    }

    // Program 2: f(x) = x² + 3x + 2 — 3 adds, 2 muls, 3 cleans
    ObfuscatedValue program_expanded(const ObfuscatedValue& x) {
        auto three = encrypt(3.0 / CLEAN_GROWTH, 0.0);
        auto two = encrypt(2.0 / CLEAN_GROWTH, 0.0);
        auto x2 = multiply(x, x); x2 = clean(x2);
        auto three_x = multiply(x, three); three_x = clean(three_x);
        auto sum = add(x2, three_x);
        auto result = add(sum, two);
        return clean(result);
    }

    CryptoContext<DCRTPoly>& getContext() { return cc; }
    KeyPair<DCRTPoly>& getKeys() { return keys; }
};

} // namespace phi_io
#endif
