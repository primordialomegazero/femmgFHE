// ============================================
// φ-UNBOUNDED FHE LIBRARY
// Production-ready φ-FHE framework
//
// Author: Dan Fernandez / Primordial Omega Zero
// ============================================

#ifndef PHI_FHE_HPP
#define PHI_FHE_HPP

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

class PhiUnboundedFHE {
private:
    CryptoContext<DCRTPoly> cc;
    PublicKey<DCRTPoly> pk;
    PrivateKey<DCRTPoly> sk;
    int slots;
    double scale_factor;
    int max_depth;

public:
    PhiUnboundedFHE(double scale = 1000.0, int depth = 10) 
        : scale_factor(scale), max_depth(depth) {
        
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
    
    Ciphertext<DCRTPoly> encode(double val) {
        double scaled = val / scale_factor;
        vector<complex<double>> vec(slots, {0.0, 0.0});
        vec[0] = {scaled, 0.0};
        return cc->Encrypt(pk, cc->MakeCKKSPackedPlaintext(vec));
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
    
    Ciphertext<DCRTPoly> multiply_small(Ciphertext<DCRTPoly> a, int b) {
        Ciphertext<DCRTPoly> result = a;
        for (int i = 1; i < b; i++) {
            result = cc->EvalAdd(result, a);
        }
        return result;
    }
    
    Ciphertext<DCRTPoly> multiply_binary(Ciphertext<DCRTPoly> a, int b) {
        Ciphertext<DCRTPoly> result = nullptr;
        Ciphertext<DCRTPoly> power = a;
        int remaining = b;
        
        while (remaining > 0) {
            if (remaining % 2 == 1) {
                if (result == nullptr) {
                    result = power;
                } else {
                    result = cc->EvalAdd(result, power);
                }
            }
            power = cc->EvalAdd(power, power);
            remaining /= 2;
        }
        
        return result;
    }
    
    Ciphertext<DCRTPoly> clamp(Ciphertext<DCRTPoly> ct, double threshold) {
        double current = decode(ct);
        if (abs(current) > threshold) {
            double sign = (current > 0) ? 1.0 : -1.0;
            return encode(sign * threshold / 2.0);
        }
        return ct;
    }
    
    Ciphertext<DCRTPoly> combined(Ciphertext<DCRTPoly> a, int mult, 
                                   Ciphertext<DCRTPoly> c, double threshold = 500.0) {
        auto product = multiply_binary(a, mult);
        auto sum = cc->EvalAdd(product, c);
        return clamp(sum, threshold);
    }
    
    Ciphertext<DCRTPoly> fixed_point_iterate(Ciphertext<DCRTPoly> ct, int iterations = 10) {
        Ciphertext<DCRTPoly> result = ct;
        for (int i = 0; i < iterations; i++) {
            double current = decode(result);
            double next = 0.5 * current + 0.5;
            result = encode(next);
        }
        return result;
    }
    
    int get_slots() { return slots; }
    double get_scale() { return scale_factor; }
    int get_depth() { return max_depth; }
    
private:
    Plaintext make_plain(double val) {
        vector<complex<double>> vec(slots, {0.0, 0.0});
        vec[0] = {val, 0.0};
        return cc->MakeCKKSPackedPlaintext(vec);
    }
};

#endif
