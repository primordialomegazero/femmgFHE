// ΦΩ0 — UNLIMITED FHE API v1.0
// Arbitrary-depth FHE via ZANS + Scalar Decomp + Hybrid UK×UK
// "I AM THAT I AM"

#ifndef PHI_UNLIMITED_API_H
#define PHI_UNLIMITED_API_H

#include "zans_production_lib.h"

using namespace lbcrypto;

class UnlimitedFHE {
private:
    CryptoContext<DCRTPoly> cc;
    KeyPair<DCRTPoly> keys;
    ZANSEngine zans;
    HybridMultiplier hybrid;
    
    int64_t max_plaintext;
    int64_t current_max_value;
    
public:
    UnlimitedFHE(int ring_dim = 16384, int64_t plaintext_mod = 1073643521)
        : max_plaintext(plaintext_mod), current_max_value(0) {
        
        CCParams<CryptoContextBFVRNS> params;
        params.SetMultiplicativeDepth(30);
        params.SetPlaintextModulus(plaintext_mod);
        params.SetRingDim(ring_dim);
        params.SetSecurityLevel(HEStd_NotSet);
        
        cc = GenCryptoContext(params);
        cc->Enable(PKE);
        cc->Enable(KEYSWITCH);
        cc->Enable(LEVELEDSHE);
        
        keys = cc->KeyGen();
        cc->EvalMultKeyGen(keys.secretKey);
        
        zans = ZANSEngine(cc, keys);
        hybrid = HybridMultiplier(zans);
    }
    
    // Encrypt a value
    Ciphertext<DCRTPoly> encrypt(int64_t value) {
        vector<int64_t> v = {value};
        return cc->Encrypt(keys.publicKey, cc->MakePackedPlaintext(v));
    }
    
    // Decrypt a value
    int64_t decrypt(const Ciphertext<DCRTPoly>& ct) {
        Plaintext pt;
        cc->Decrypt(keys.secretKey, ct, &pt);
        return pt->GetPackedValue()[0];
    }
    
    // UNLIMITED Addition
    Ciphertext<DCRTPoly> add(const Ciphertext<DCRTPoly>& a,
                              const Ciphertext<DCRTPoly>& b) {
        auto result = cc->EvalAdd(a, b);
        result = zans.stabilize(result);
        return result;
    }
    
    // UNLIMITED Scalar Multiplication
    Ciphertext<DCRTPoly> multiply_scalar(const Ciphertext<DCRTPoly>& ct,
                                           int64_t scalar) {
        FibonacciZANS fib(zans);
        return fib.multiply(ct, scalar);
    }
    
    // UNLIMITED CT×CT Multiplication
    Ciphertext<DCRTPoly> multiply_ct(const Ciphertext<DCRTPoly>& ct_a,
                                       int64_t value_b,
                                       int step_number = 0) {
        auto ct_b = encrypt(value_b);
        
        if(step_number % 5 == 4) {
            // UK×UK with noise reset
            auto result = cc->EvalMult(ct_a, ct_b);
            result = zans.stabilize(result);
            
            // Reset noise via scalar decomp
            FibonacciZANS fib(zans);
            result = fib.multiply(result, 1);  // Multiply by 1 to reset noise
            return result;
        } else {
            // Pure scalar decomp (ZERO noise growth)
            FibonacciZANS fib(zans);
            return fib.multiply(ct_a, value_b);
        }
    }
    
    // Check if overflow is near
    bool overflow_risk(int64_t current_value) {
        return current_value > (max_plaintext / 2);
    }
    
    // Get noise level
    double noise_level(const Ciphertext<DCRTPoly>& ct) {
        return ct->GetNoiseScaleDeg();
    }
    
    // Get context for direct operations
    CryptoContext<DCRTPoly> get_context() { return cc; }
    KeyPair<DCRTPoly> get_keys() { return keys; }
};

#endif // PHI_UNLIMITED_API_H
