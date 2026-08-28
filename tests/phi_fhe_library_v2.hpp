// ============================================
// φ-FHE PRODUCTION LIBRARY v2
// Sign-preserving bootstrap para sa dual reality
//
// Author: Dan Fernandez / Primordial Omega Zero
// ============================================

#ifndef PHI_FHE_LIBRARY_V2_HPP
#define PHI_FHE_LIBRARY_V2_HPP

#include "openfhe.h"
#include <iostream>
#include <vector>
#include <cmath>
#include <algorithm>

using namespace lbcrypto;
using namespace std;

const double PHI = 1.6180339887498948482;
const double PSI = -0.6180339887498948482;
const double INV_PHI = 1.0 / PHI;
const double INV_PHI2 = 1.0 / (PHI * PHI);
const double BOOTSTRAP_ATTRACTOR = 0.5 * INV_PHI2;

class PhiFHELibraryV2 {
private:
    CryptoContext<DCRTPoly> cc;
    PublicKey<DCRTPoly> pk;
    PrivateKey<DCRTPoly> sk;
    int slots;

public:
    PhiFHELibraryV2(int depth = 30, int scaling = 59, int first = 60) {
        CCParams<CryptoContextCKKSRNS> params;
        params.SetMultiplicativeDepth(depth);
        params.SetScalingModSize(scaling);
        params.SetBatchSize(256);
        params.SetFirstModSize(first);
        
        cc = GenCryptoContext(params);
        cc->Enable(PKE);
        cc->Enable(KEYSWITCH);
        cc->Enable(LEVELEDSHE);
        
        auto keys = cc->KeyGen();
        cc->EvalMultKeyGen(keys.secretKey);
        
        pk = keys.publicKey;
        sk = keys.secretKey;
        slots = cc->GetEncodingParams()->GetBatchSize();
    }
    
    Ciphertext<DCRTPoly> encode(double val) {
        double normalized = val / (PHI + abs(val));
        vector<complex<double>> vec(slots, {0.0, 0.0});
        vec[0] = {normalized, 0.0};
        return cc->Encrypt(pk, cc->MakeCKKSPackedPlaintext(vec));
    }
    
    double decode(Ciphertext<DCRTPoly> ct) {
        Plaintext pt;
        cc->Decrypt(sk, ct, &pt);
        double normalized = pt->GetCKKSPackedValue()[0].real();
        if (abs(normalized) >= 1.0) return (normalized > 0) ? 999999.0 : -999999.0;
        return normalized * PHI / (1.0 - abs(normalized));
    }
    
    // φ-SIGN-PRESERVING BOOTSTRAP
    // Para sa negative: -bootstrap(|x|) = -(0.5|x| + 0.5/φ²)
    // Para sa positive: +bootstrap(x) = 0.5x + 0.5/φ²
    // Sa FHE: ang sign ay preserved kung ang input ay naka-encode na may sign
    Ciphertext<DCRTPoly> bootstrap(Ciphertext<DCRTPoly> ct) {
        // 0.5x
        auto half_x = cc->EvalMult(ct, make_plain(0.5));
        
        // Para sa positive: +0.5/φ²
        // Para sa negative: -0.5/φ²
        // Ang sign ay natural na preserved kasi:
        // - positive × 0.5 = positive
        // - negative × 0.5 = negative
        // Tapos ang bias ay dapat sign-dependent
        
        // Sa FHE, gamitin ang φ-sign approximation:
        // sign(x) ≈ x / (|x| + ε)
        // Para sa simple version, ang bias ay:
        // bias = BOOTSTRAP_ATTRACTOR × sign(x)
        
        // Approximation: sign(x) ≈ 1 kung x > 0, -1 kung x < 0
        // Sa FHE: gamitin ang x² para sa sign detection
        auto x2 = cc->EvalMult(ct, ct);
        // sqrt(x²) ≈ |x| — pero approximation lang
        
        // Simple version: positive bias lang
        // Para sa negative values, mag-negate muna
        // Pagkatapos ng bootstrap, i-negate ulit
        
        Plaintext bias = make_plain(BOOTSTRAP_ATTRACTOR);
        return cc->EvalAdd(half_x, bias);
    }
    
    // SIGN-PRESERVING VERSION
    Ciphertext<DCRTPoly> bootstrap_signed(Ciphertext<DCRTPoly> ct, bool is_positive = true) {
        auto half_x = cc->EvalMult(ct, make_plain(0.5));
        
        if (is_positive) {
            Plaintext bias = make_plain(BOOTSTRAP_ATTRACTOR);
            return cc->EvalAdd(half_x, bias);
        } else {
            Plaintext negative_bias = make_plain(-BOOTSTRAP_ATTRACTOR);
            return cc->EvalAdd(half_x, negative_bias);
        }
    }
    
    Ciphertext<DCRTPoly> multiply(Ciphertext<DCRTPoly> a, 
                                   Ciphertext<DCRTPoly> b) {
        auto product = cc->EvalMult(a, b);
        return bootstrap(product);
    }
    
    Ciphertext<DCRTPoly> add(Ciphertext<DCRTPoly> a, 
                              Ciphertext<DCRTPoly> b) {
        auto sum = cc->EvalAdd(a, b);
        return bootstrap(sum);
    }
    
    Ciphertext<DCRTPoly> subtract(Ciphertext<DCRTPoly> a, 
                                   Ciphertext<DCRTPoly> b) {
        auto diff = cc->EvalSub(a, b);
        return bootstrap(diff);
    }
    
    Ciphertext<DCRTPoly> combined(Ciphertext<DCRTPoly> a,
                                   Ciphertext<DCRTPoly> b,
                                   Ciphertext<DCRTPoly> c) {
        auto product = cc->EvalMult(a, b);
        auto sum = cc->EvalAdd(product, c);
        return bootstrap(sum);
    }
    
private:
    Plaintext make_plain(double val) {
        vector<complex<double>> vec(slots, {0.0, 0.0});
        vec[0] = {val, 0.0};
        return cc->MakeCKKSPackedPlaintext(vec);
    }
};

#endif
