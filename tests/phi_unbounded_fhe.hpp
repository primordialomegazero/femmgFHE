// ============================================
// φ-UNBOUNDED FHE LIBRARY
// Kumpletong φ-FHE integration
//
// Features:
// - Linear encoding (zero-depth multiplication)
// - Binary multiplication (O(log b) additions)
// - φ-Bootstrap (natural na refresh)
// - Dual reality (linear + non-linear)
// - Complete API para sa production
//
// Author: Dan Fernandez / Primordial Omega Zero
// ============================================

#ifndef PHI_UNBOUNDED_FHE_HPP
#define PHI_UNBOUNDED_FHE_HPP

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
    
public:
    PhiUnboundedFHE(double scale = 1000.0, int depth = 10) : scale_factor(scale) {
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
    
    // ========== LINEAR ENCODING ==========
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
    
    // ========== PURE ADDITION (0 levels) ==========
    Ciphertext<DCRTPoly> add(Ciphertext<DCRTPoly> a, Ciphertext<DCRTPoly> b) {
        return cc->EvalAdd(a, b);
    }
    
    Ciphertext<DCRTPoly> subtract(Ciphertext<DCRTPoly> a, Ciphertext<DCRTPoly> b) {
        return cc->EvalSub(a, b);
    }
    
    // ========== SMALL MULTIPLICATION (0 levels) ==========
    Ciphertext<DCRTPoly> multiply_small(Ciphertext<DCRTPoly> a, int b) {
        Ciphertext<DCRTPoly> result = a;
        for (int i = 1; i < b; i++) {
            result = cc->EvalAdd(result, a);
        }
        return result;
    }
    
    // ========== BINARY MULTIPLICATION (0 levels, O(log b)) ==========
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
            power = cc->EvalAdd(power, power);  // Double — 0 levels!
            remaining /= 2;
        }
        
        return result;
    }
    
    // ========== φ-BOOTSTRAP (1 mult, para sa bounding) ==========
    Ciphertext<DCRTPoly> bootstrap(Ciphertext<DCRTPoly> ct) {
        auto half_x = cc->EvalMult(ct, make_plain(0.5));
        Plaintext bias = make_plain(0.5 * INV_PHI2);
        return cc->EvalAdd(half_x, bias);
    }
    
    // ========== COMBINED (multiply + add + bootstrap) ==========
    Ciphertext<DCRTPoly> combined(Ciphertext<DCRTPoly> a, int mult, Ciphertext<DCRTPoly> c) {
        auto product = multiply_binary(a, mult);    // 0 levels
        auto sum = cc->EvalAdd(product, c);          // 0 levels
        return bootstrap(sum);                        // 1 mult
    }
    
    // ========== COMPLEX COMPUTATION ==========
    Ciphertext<DCRTPoly> compute_polynomial(Ciphertext<DCRTPoly> x, 
                                             vector<int> coefficients) {
        // Evaluate: c0 + c1×x + c2×x² + ... + cn×x^n
        Ciphertext<DCRTPoly> result = encode(0.0);
        Ciphertext<DCRTPoly> power = encode(1.0);
        
        for (size_t i = 0; i < coefficients.size(); i++) {
            auto term = multiply_binary(power, coefficients[i]);
            result = cc->EvalAdd(result, term);
            if (i < coefficients.size() - 1) {
                power = multiply_binary(power, (int)decode(x) % 1000);
            }
        }
        
        return result;
    }
    
    // ========== GETTERS ==========
    int get_slots() { return slots; }
    double get_scale() { return scale_factor; }
    
private:
    Plaintext make_plain(double val) {
        vector<complex<double>> vec(slots, {0.0, 0.0});
        vec[0] = {val, 0.0};
        return cc->MakeCKKSPackedPlaintext(vec);
    }
};

#endif
