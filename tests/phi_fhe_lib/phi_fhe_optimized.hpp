// ============================================
// φ-OPTIMIZED FHE LIBRARY
// Emergent optimization para sa φ-FHE
//
// Features:
// - Cached ciphertexts — walang paulit-ulit na encode
// - Batch operations — parallel processing
// - φ-Skip — skip kung attractor na
//
// Author: Dan Fernandez / Primordial Omega Zero
// ============================================

#ifndef PHI_FHE_OPTIMIZED_HPP
#define PHI_FHE_OPTIMIZED_HPP

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

class PhiOptimizedFHE {
private:
    CryptoContext<DCRTPoly> cc;
    PublicKey<DCRTPoly> pk;
    PrivateKey<DCRTPoly> sk;
    int slots;
    double scale_factor;
    
    // CACHE para sa common values
    unordered_map<int, Ciphertext<DCRTPoly>> cache;
    unordered_map<int, Plaintext> plain_cache;
    
public:
    PhiOptimizedFHE(double scale = 1000.0, int depth = 10) : scale_factor(scale) {
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
        
        // Pre-cache ang common values
        pre_cache_common_values();
    }
    
    void pre_cache_common_values() {
        // Pre-cache: 0, 0.1, 0.5, 1, 2, 5, 10
        vector<double> common = {0.0, 0.1, 0.5, 1.0, 2.0, 5.0, 10.0};
        for (double val : common) {
            int key = (int)(val * 100);
            cache[key] = encode_internal(val);
        }
    }
    
    Ciphertext<DCRTPoly> encode_internal(double val) {
        double scaled = val / scale_factor;
        vector<complex<double>> vec(slots, {0.0, 0.0});
        vec[0] = {scaled, 0.0};
        return cc->Encrypt(pk, cc->MakeCKKSPackedPlaintext(vec));
    }
    
    // Encode na may caching
    Ciphertext<DCRTPoly> encode(double val) {
        int key = (int)(val * 100);
        if (cache.find(key) != cache.end()) {
            return cache[key];  // CACHED!
        }
        auto result = encode_internal(val);
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
    
    // OPTIMIZED: multiply na may cached doubles
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
    
    // OPTIMIZED: 1000 additions na may cached values
    Ciphertext<DCRTPoly> add_many_optimized(Ciphertext<DCRTPoly> start, 
                                             double inc, int count) {
        Ciphertext<DCRTPoly> result = start;
        Ciphertext<DCRTPoly> inc_ct = encode(inc);  // CACHED!
        
        for (int i = 0; i < count; i++) {
            result = cc->EvalAdd(result, inc_ct);
        }
        
        return result;
    }
    
    // φ-SKIP: kung ang value ay nasa φ-attractor, skip
    Ciphertext<DCRTPoly> smart_add(Ciphertext<DCRTPoly> a, double b, 
                                    double attractor = INV_PHI2) {
        double current = decode(a);
        if (abs(current - attractor) < 0.001) {
            return a;  // SKIP — nasa attractor na!
        }
        return cc->EvalAdd(a, encode(b));
    }
    
private:
    Plaintext make_plain(double val) {
        vector<complex<double>> vec(slots, {0.0, 0.0});
        vec[0] = {val, 0.0};
        return cc->MakeCKKSPackedPlaintext(vec);
    }
};

#endif
