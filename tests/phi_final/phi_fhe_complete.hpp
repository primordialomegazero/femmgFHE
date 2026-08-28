// ============================================
// φ-FHE COMPLETE LIBRARY — FINAL VERSION
// Production-ready na may lahat ng features
//
// Author: Dan Fernandez / Primordial Omega Zero
// Version: 2.0.0
// ============================================

#ifndef PHI_FHE_COMPLETE_HPP
#define PHI_FHE_COMPLETE_HPP

#include "openfhe.h"
#include <iostream>
#include <vector>
#include <cmath>
#include <algorithm>
#include <unordered_map>

using namespace lbcrypto;
using namespace std;

const double PHI = 1.6180339887498948482;
const double PSI = -0.6180339887498948482;
const double INV_PHI = 1.0 / PHI;
const double INV_PHI2 = 1.0 / (PHI * PHI);

class PhiCompleteFHE {
private:
    CryptoContext<DCRTPoly> cc;
    PublicKey<DCRTPoly> pk;
    PrivateKey<DCRTPoly> sk;
    int slots;
    double scale_factor;
    unordered_map<int, Ciphertext<DCRTPoly>> cache;
    
public:
    PhiCompleteFHE(double scale = 100.0, int depth = 5) : scale_factor(scale) {
        CCParams<CryptoContextCKKSRNS> params;
        params.SetMultiplicativeDepth(depth);
        params.SetScalingModSize(59);
        params.SetBatchSize(256);
        params.SetFirstModSize(60);
        
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
    
    // ENCODE na may caching
    Ciphertext<DCRTPoly> encode(double val) {
        int key = (int)(val * 100);
        if (cache.find(key) != cache.end()) return cache[key];
        
        double scaled = val / scale_factor;
        vector<complex<double>> vec(slots, {0.0, 0.0});
        vec[0] = {scaled, 0.0};
        auto result = cc->Encrypt(pk, cc->MakeCKKSPackedPlaintext(vec));
        cache[key] = result;
        return result;
    }
    
    double decode(Ciphertext<DCRTPoly> ct) {
        Plaintext pt;
        cc->Decrypt(sk, ct, &pt);
        return pt->GetCKKSPackedValue()[0].real() * scale_factor;
    }
    
    Ciphertext<DCRTPoly> add(Ciphertext<DCRTPoly> a, Ciphertext<DCRTPoly> b) {
        return cc->EvalAdd(a, b);
    }
    
    Ciphertext<DCRTPoly> subtract(Ciphertext<DCRTPoly> a, Ciphertext<DCRTPoly> b) {
        return cc->EvalSub(a, b);
    }
    
    // BINARY MULTIPLICATION — ZERO EvalMult!
    Ciphertext<DCRTPoly> multiply(Ciphertext<DCRTPoly> a, int b) {
        Ciphertext<DCRTPoly> result = nullptr;
        Ciphertext<DCRTPoly> power = a;
        int remaining = b;
        
        while (remaining > 0) {
            if (remaining % 2 == 1) {
                if (result == nullptr) result = power;
                else result = cc->EvalAdd(result, power);
            }
            power = cc->EvalAdd(power, power);
            remaining /= 2;
        }
        return result;
    }
    
    // MULTIPLY WITH ATTRACTOR RESET
    Ciphertext<DCRTPoly> multiply_reset(Ciphertext<DCRTPoly> a, int b) {
        auto product = multiply(a, b);
        double current = decode(product);
        if (abs(current) > scale_factor) {
            double sign = (current > 0) ? 1.0 : -1.0;
            return encode(sign * scale_factor * INV_PHI2);
        }
        return product;
    }
    
private:
    Plaintext make_plain(double val) {
        vector<complex<double>> vec(slots, {0.0, 0.0});
        vec[0] = {val, 0.0};
        return cc->MakeCKKSPackedPlaintext(vec);
    }
};

#endif
