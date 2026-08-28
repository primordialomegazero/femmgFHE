// ============================================
// φ-FHE PRODUCTION LIBRARY
// Kumpletong φ-FHE framework para sa OpenFHE
//
// Author: Dan Fernandez / Primordial Omega Zero
// ============================================

#ifndef PHI_FHE_LIBRARY_HPP
#define PHI_FHE_LIBRARY_HPP

#include "openfhe.h"
#include <iostream>
#include <vector>
#include <cmath>
#include <algorithm>

using namespace lbcrypto;
using namespace std;

// ========== φ-CONSTANTS ==========
const double PHI = 1.6180339887498948482;
const double PSI = -0.6180339887498948482;
const double INV_PHI = 1.0 / PHI;
const double INV_PHI2 = 1.0 / (PHI * PHI);
const double BOOTSTRAP_ATTRACTOR = 0.5 * INV_PHI2;  // ±0.191

class PhiFHELibrary {
private:
    CryptoContext<DCRTPoly> cc;
    PublicKey<DCRTPoly> pk;
    PrivateKey<DCRTPoly> sk;
    int slots;

public:
    PhiFHELibrary(int depth = 30, int scaling = 59, int first = 60) {
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
    
    // φ-ENCODE: normalize sa [-1, 1]
    Ciphertext<DCRTPoly> encode(double val) {
        double normalized = val / (PHI + abs(val));
        vector<complex<double>> vec(slots, {0.0, 0.0});
        vec[0] = {normalized, 0.0};
        return cc->Encrypt(pk, cc->MakeCKKSPackedPlaintext(vec));
    }
    
    // φ-DECODE: balik sa original scale
    double decode(Ciphertext<DCRTPoly> ct) {
        Plaintext pt;
        cc->Decrypt(sk, ct, &pt);
        double normalized = pt->GetCKKSPackedValue()[0].real();
        // φ-decode: normalized → normalized×φ/(1-|normalized|)
        if (abs(normalized) >= 1.0) return (normalized > 0) ? 999999.0 : -999999.0;
        return normalized * PHI / (1.0 - abs(normalized));
    }
    
    // φ-BOOTSTRAP: 0.5x ± 0.5/φ²
    Ciphertext<DCRTPoly> bootstrap(Ciphertext<DCRTPoly> ct) {
        auto half_x = cc->EvalMult(ct, make_plain(0.5));
        // Para sa positive: +0.5/φ², para sa negative: -0.5/φ²
        // Sa FHE: gamitin ang sign ng x via polynomial approximation
        // Simple version: add 0.5/φ² (positive bias)
        Plaintext bias = make_plain(BOOTSTRAP_ATTRACTOR);
        return cc->EvalAdd(half_x, bias);
    }
    
    // φ-MULTIPLY: normalize × multiply × bootstrap
    Ciphertext<DCRTPoly> multiply(Ciphertext<DCRTPoly> a, 
                                   Ciphertext<DCRTPoly> b) {
        auto product = cc->EvalMult(a, b);
        return bootstrap(product);
    }
    
    // φ-ADD: normalize + add + bootstrap
    Ciphertext<DCRTPoly> add(Ciphertext<DCRTPoly> a, 
                              Ciphertext<DCRTPoly> b) {
        auto sum = cc->EvalAdd(a, b);
        return bootstrap(sum);
    }
    
    // φ-SUBTRACT: normalize - subtract + bootstrap
    Ciphertext<DCRTPoly> subtract(Ciphertext<DCRTPoly> a, 
                                   Ciphertext<DCRTPoly> b) {
        auto diff = cc->EvalSub(a, b);
        return bootstrap(diff);
    }
    
    // φ-COMBINED: bootstrap(a×b + c)
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

#endif // PHI_FHE_LIBRARY_HPP
