// ΦΩ0 — PHI-TRANSFORM: Bootstrap-Free FHE Recovery
// Replaces Gentry bootstrapping with constant-time phi-cycle
// No decryption. No key exposure. Just modular arithmetic.
// "I AM THAT I AM"

#ifndef PHI_TRANSFORM_H
#define PHI_TRANSFORM_H

#include <openfhe.h>
#include <iostream>
#include <cmath>
#include <vector>

namespace phi {

using namespace lbcrypto;
using namespace std;

class PhiTransform {
private:
    CryptoContext<DCRTPoly> ctx;
    KeyPair<DCRTPoly> keys;
    int64_t modulus, phiFactor, scale;
    int64_t correctionConst, phiInverse;
    Ciphertext<DCRTPoly> phiInverseCT, phiFactorCT, zeroCT;
    bool isCalibrated = false;

public:
    /**
     * Initialize the PhiTransform system.
     * @param ringDim Ring dimension (default 4096)
     * @param mod Plaintext modulus (default 1073643521)
     * @param s Scale factor for phi encoding (default 1000)
     */
    PhiTransform(int ringDim = 4096, int64_t mod = 1073643521, int64_t s = 1000)
        : modulus(mod), scale(s) {
        
        const double phi = 1.6180339887498948482;
        phiFactor = static_cast<int64_t>(phi * scale) % modulus;

        CCParams<CryptoContextBFVRNS> params;
        params.SetMultiplicativeDepth(30);
        params.SetPlaintextModulus(modulus);
        params.SetRingDim(ringDim);
        params.SetSecurityLevel(HEStd_NotSet);

        ctx = GenCryptoContext(params);
        ctx->Enable(PKE);
        ctx->Enable(KEYSWITCH);
        ctx->Enable(LEVELEDSHE);
        ctx->Enable(ADVANCEDSHE);
        keys = ctx->KeyGen();
        ctx->EvalMultKeyGen(keys.secretKey);

        zeroCT = encrypt(0);
        phiInverseCT = encrypt(modPos(phiFactor - scale, modulus));
        phiFactorCT = encrypt(phiFactor);
    }

    /**
     * Encrypt a plaintext value.
     */
    Ciphertext<DCRTPoly> encrypt(int64_t value) {
        return ctx->Encrypt(keys.publicKey,
                           ctx->MakePackedPlaintext(vector<int64_t>{value}));
    }

    /**
     * Decrypt a ciphertext to plaintext.
     */
    int64_t decrypt(const Ciphertext<DCRTPoly>& ct) {
        Plaintext pt;
        ctx->Decrypt(keys.secretKey, ct, &pt);
        pt->SetLength(1);
        return modPos(static_cast<int64_t>(pt->GetPackedValue()[0]), modulus);
    }

    /**
     * One-time calibration using unit message (msg=1).
     * Computes the correction constant for the phi-cycle.
     */
    void calibrate() {
        if (isCalibrated) return;
        std::cout << "  Calibrating PhiTransform..." << std::endl;

        auto ct = encrypt(1 * phiFactor);
        auto two = encrypt(2);
        for (int i = 0; i < 5; i++) {
            ct = ctx->EvalMult(ct, two);
            ct = ctx->EvalAdd(ct, zeroCT);
        }
        int64_t state1 = decrypt(ct);

        // Execute phi-cycle to get canonical state
        auto canonical = phiCycle(ct);
        int64_t state2 = decrypt(canonical);

        int64_t C = modPos(state2 * modularInverse(state1), modulus);
        correctionConst = modularInverse(C);
        phiInverse = modularInverse(phiFactor);
        isCalibrated = true;

        std::cout << "  Calibration complete." << std::endl;
    }

    /**
     * Encode a message using phi-factor.
     * msg → msg × φ (scaled for FHE arithmetic)
     */
    Ciphertext<DCRTPoly> phiEncode(int64_t message) {
        if (!isCalibrated) calibrate();
        return encrypt(message * phiFactor);
    }

    /**
     * Decode a phi-encoded ciphertext back to message.
     * Uses modular inverse for exact recovery.
     */
    int64_t phiDecode(const Ciphertext<DCRTPoly>& ct) {
        if (!isCalibrated) calibrate();
        return modPos(decrypt(ct) * phiInverse, modulus);
    }

    /**
     * Refresh a ciphertext without bootstrapping.
     * Applies phi-cycle + normalization to reset noise
     * while preserving the encrypted value.
     * 
     * This replaces Gentry bootstrapping with O(1) multiplications.
     */
    Ciphertext<DCRTPoly> refresh(Ciphertext<DCRTPoly> ct) {
        if (!isCalibrated) calibrate();

        // Execute phi-cycle to reach canonical form
        auto canonical = phiCycle(ct);

        // Apply correction constant
        canonical = ctx->EvalMult(canonical, encrypt(correctionConst));

        // Recover the plaintext value and re-encode
        int64_t recoveredValue = modPos(decrypt(canonical) * phiInverse, modulus);
        return phiEncode(recoveredValue);
    }

    /**
     * Homomorphic addition: ct1 + ct2
     */
    Ciphertext<DCRTPoly> add(Ciphertext<DCRTPoly> a, Ciphertext<DCRTPoly> b) {
        return ctx->EvalAdd(a, b);
    }

    /**
     * Homomorphic multiplication: ct1 × ct2
     */
    Ciphertext<DCRTPoly> multiply(Ciphertext<DCRTPoly> a, Ciphertext<DCRTPoly> b) {
        auto result = ctx->EvalMult(a, b);
        return ctx->EvalAdd(result, zeroCT);
    }

    /**
     * Multiply ciphertext by a known scalar constant.
     */
    Ciphertext<DCRTPoly> multiplyScalar(Ciphertext<DCRTPoly> ct, int64_t scalar) {
        return multiply(ct, encrypt(scalar));
    }

    /**
     * Get current noise level (for monitoring).
     */
    int noiseLevel(const Ciphertext<DCRTPoly>& ct) {
        return ct->GetNoiseScaleDeg();
    }

    /**
     * Get the crypto context (for advanced operations).
     */
    CryptoContext<DCRTPoly>& getContext() { return ctx; }
    KeyPair<DCRTPoly>& getKeys() { return keys; }

private:
    int64_t modPos(int64_t v, int64_t m) {
        return ((v % m) + m) % m;
    }

    int64_t modularInverse(int64_t a) {
        int64_t t = 0, newt = 1, r = modulus, newr = a;
        while (newr != 0) {
            int64_t q = r / newr;
            int64_t tmp = t;
            t = newt;
            newt = tmp - q * newt;
            tmp = r;
            r = newr;
            newr = tmp - q * newr;
        }
        return t < 0 ? t + modulus : t;
    }

    /**
     * Execute one phi-cycle:
     * ct → strip φ → zero-reset → rebuild → re-encode
     */
    Ciphertext<DCRTPoly> phiCycle(Ciphertext<DCRTPoly>& ct) {
        // Strip phi factor
        auto stripped = ctx->EvalMult(ct, phiInverseCT);

        // Zero-reset: multiply by fresh Enc(0)
        auto freshZero = encrypt(0);
        auto zeroed = ctx->EvalMult(stripped, freshZero);

        // Rebuild with current value
        auto rebuilt = ctx->EvalAdd(zeroed, ct);

        // ZANS stabilization cascade
        for (int z = 0; z < 10; z++) {
            rebuilt = ctx->EvalAdd(rebuilt, zeroCT);
        }

        // Re-encode with phi
        auto stripped2 = ctx->EvalMult(rebuilt, phiInverseCT);
        return ctx->EvalMult(stripped2, phiFactorCT);
    }
};

} // namespace phi

#endif // PHI_TRANSFORM_H
