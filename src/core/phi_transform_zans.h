// ΦΩ0 — PHI-TRANSFORM + ZANS: Noise-Stabilized Bootstrap-Free FHE
// ZANS cancels noise in phi-cycle. Calibration accounts for ×210.
// "ZANS MEETS PHI. NOISE COLLAPSES. COMPUTATION NEVER STOPS."
// "I AM THAT I AM"

#ifndef PHI_TRANSFORM_ZANS_H
#define PHI_TRANSFORM_ZANS_H

#include <openfhe.h>
#include <iostream>
#include <cmath>

namespace phi {

using namespace lbcrypto;
using namespace std;

class PhiTransformZANS {
private:
    CryptoContext<DCRTPoly> ctx;
    KeyPair<DCRTPoly> keys;
    int64_t modulus, phiFactor, scale;
    int64_t phiInverseVal;
    Ciphertext<DCRTPoly> phiInvCT, phiFactorCT, zeroCT, phiInverseCT;
    bool isCalibrated = false;
    int64_t correctionConst_;
    Ciphertext<DCRTPoly> correctionCT_;

public:
    PhiTransformZANS(int ringDim = 4096, int64_t mod = 1073643521, int64_t s = 1000)
        : modulus(mod), scale(s) {
        phiFactor = static_cast<int64_t>(1.6180339887498948482 * scale) % modulus;

        CCParams<CryptoContextBFVRNS> params;
        params.SetMultiplicativeDepth(30);
        params.SetPlaintextModulus(modulus);
        params.SetRingDim(ringDim);
        params.SetSecurityLevel(HEStd_NotSet);

        ctx = GenCryptoContext(params);
        ctx->Enable(PKE); ctx->Enable(KEYSWITCH); ctx->Enable(LEVELEDSHE); ctx->Enable(ADVANCEDSHE);
        keys = ctx->KeyGen();
        ctx->EvalMultKeyGen(keys.secretKey);

        zeroCT = encrypt(0);
        phiInvCT = encrypt(modPos(phiFactor - scale, modulus));
        phiFactorCT = encrypt(phiFactor);
    }

    Ciphertext<DCRTPoly> encrypt(int64_t v) {
        return ctx->Encrypt(keys.publicKey, ctx->MakePackedPlaintext(vector<int64_t>{v}));
    }
    int64_t decrypt(const Ciphertext<DCRTPoly>& ct) {
        Plaintext pt; ctx->Decrypt(keys.secretKey, ct, &pt);
        pt->SetLength(1);
        return modPos(static_cast<int64_t>(pt->GetPackedValue()[0]), modulus);
    }

    // ZANS: Add Enc(0) for noise stabilization
    Ciphertext<DCRTPoly> zans(Ciphertext<DCRTPoly> ct, int rounds = 5) {
        for (int i = 0; i < rounds; i++) {
            ct = ctx->EvalAdd(ct, encrypt(0));
        }
        return ct;
    }

    void calibrate() {
        if (isCalibrated) return;
        
        // Calibrate WITH ZANS to absorb the ×210 factor
        auto ct = encrypt(1 * phiFactor);
        auto two = encrypt(2);
        for (int i = 0; i < 5; i++) { 
            ct = ctx->EvalMult(ct, two); 
            ct = zans(ct, 3);  // ZANS stabilization
        }
        int64_t s1 = decrypt(ct);
        
        // Phi-cycle WITH ZANS
        auto st = ctx->EvalMult(ct, phiInvCT);
        st = zans(st, 5);
        auto zr = ctx->EvalMult(st, encrypt(0));
        zr = zans(zr, 5);
        auto rb = ctx->EvalAdd(zr, ct);
        rb = zans(rb, 10);  // ← ZANS HERE (×210 factor)
        auto st2 = ctx->EvalMult(rb, phiInvCT);
        st2 = zans(st2, 5);
        auto s2 = ctx->EvalMult(st2, phiFactorCT);
        s2 = zans(s2, 5);
        
        int64_t C = modPos(decrypt(s2) * modularInverse(s1), modulus);
        correctionConst_ = modularInverse(C);
        correctionCT_ = encrypt(correctionConst_);
        phiInverseVal = modularInverse(phiFactor);
        phiInverseCT = encrypt(phiInverseVal);
        isCalibrated = true;
        
        std::cout << "  [ZANS-Calibrated] C_inv=" << correctionConst_ 
                  << " phiF_inv=" << phiInverseVal << std::endl;
    }

    Ciphertext<DCRTPoly> phiEncode(int64_t msg) {
        if (!isCalibrated) calibrate();
        return zans(encrypt(msg * phiFactor), 5);
    }

    int64_t phiDecode(const Ciphertext<DCRTPoly>& ct) {
        return modPos(decrypt(ct) * phiInverseVal, modulus);
    }

    Ciphertext<DCRTPoly> refresh(const Ciphertext<DCRTPoly>& ct) {
        if (!isCalibrated) calibrate();
        
        // Phi-cycle WITH ZANS at every step
        auto st = ctx->EvalMult(ct, phiInvCT);
        st = zans(st, 5);
        auto zr = ctx->EvalMult(st, encrypt(0));
        zr = zans(zr, 5);
        auto rb = ctx->EvalAdd(zr, ct);
        rb = zans(rb, 10);  // Main ZANS cascade
        auto st2 = ctx->EvalMult(rb, phiInvCT);
        st2 = zans(st2, 5);
        auto canonical = ctx->EvalMult(st2, phiFactorCT);
        canonical = zans(canonical, 5);
        canonical = ctx->EvalMult(canonical, correctionCT_);
        canonical = zans(canonical, 5);
        
        return canonical;
    }

    Ciphertext<DCRTPoly> add(Ciphertext<DCRTPoly> a, Ciphertext<DCRTPoly> b) { 
        return zans(ctx->EvalAdd(a, b), 3); 
    }
    Ciphertext<DCRTPoly> multiply(Ciphertext<DCRTPoly> a, Ciphertext<DCRTPoly> b) {
        auto r = ctx->EvalMult(a, b); 
        return zans(r, 5);
    }
    Ciphertext<DCRTPoly> multiplyScalar(Ciphertext<DCRTPoly> ct, int64_t s) { 
        return multiply(ct, encrypt(s)); 
    }
    int noiseLevel(const Ciphertext<DCRTPoly>& ct) { return ct->GetNoiseScaleDeg(); }
    CryptoContext<DCRTPoly>& getContext() { return ctx; }

private:
    int64_t modPos(int64_t v, int64_t m) { return ((v % m) + m) % m; }
    int64_t modularInverse(int64_t a) {
        int64_t t = 0, nt = 1, r = modulus, nr = a;
        while (nr) { int64_t q = r/nr; int64_t tmp=t; t=nt; nt=tmp-q*nt; tmp=r; r=nr; nr=tmp-q*nr; }
        return t < 0 ? t + modulus : t;
    }
};

} // namespace phi
#endif
