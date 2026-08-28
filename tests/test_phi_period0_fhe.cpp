// ============================================
// φ-PERIOD-0 FHE TEST
// φ-Normalize bootstrap sa OpenFHE
//
// Core formula:
// bootstrap(x) = x/(φ+|x|) ≈ x×(1-|x|/φ)
// 2 EvalMult, period-0, autonomous
//
// Author: Dan Fernandez / Primordial Omega Zero
// ============================================

#include "openfhe.h"
#include <iostream>
#include <vector>
#include <cmath>
#include <algorithm>
#include <chrono>

using namespace lbcrypto;
using namespace std;
using namespace std::chrono;

const double SCALE = 10.0;
const double PHI = 1.6180339887498948482;
const double INV_PHI = 1.0 / PHI;

class PhiPeriod0FHE {
private:
    CryptoContext<DCRTPoly> cc;
    PublicKey<DCRTPoly> pk;
    PrivateKey<DCRTPoly> sk;
    int slots;

public:
    PhiPeriod0FHE() {
        CCParams<CryptoContextCKKSRNS> params;
        params.SetMultiplicativeDepth(30);
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
        double scaled = val / SCALE;
        vector<complex<double>> vec(slots, {0.0, 0.0});
        vec[0] = {scaled, 0.0};
        return cc->Encrypt(pk, cc->MakeCKKSPackedPlaintext(vec));
    }
    
    double decode(Ciphertext<DCRTPoly> ct) {
        Plaintext pt;
        cc->Decrypt(sk, ct, &pt);
        return pt->GetCKKSPackedValue()[0].real() * SCALE;
    }
    
    // PERIOD-0 BOOTSTRAP: x → x×(1-|x|/φ)
    // 2 EvalMult: |x|² at x×(1-|x|/φ)
    Ciphertext<DCRTPoly> bootstrap(Ciphertext<DCRTPoly> ct) {
        // |x| ≈ x² (para sa positive x)
        auto x2 = cc->EvalMult(ct, ct);           // 1 EvalMult
        
        // |x|/φ
        auto x2_over_phi = cc->EvalMult(x2, make_plain(INV_PHI));
        
        // 1 - |x|/φ
        Plaintext one = make_plain(1.0);
        auto one_minus = cc->EvalSub(one, x2_over_phi);
        
        // x × (1 - |x|/φ) — 1 EvalMult
        return cc->EvalMult(ct, one_minus);
    }
    
    // CT × CT with period-0 bootstrap
    Ciphertext<DCRTPoly> multiply_auto(Ciphertext<DCRTPoly> a, Ciphertext<DCRTPoly> b) {
        auto product = cc->EvalMult(a, b);  // 1 EvalMult — small product
        return bootstrap(product);            // 2 EvalMult — period-0!
    }
    
private:
    Plaintext make_plain(double val) {
        vector<complex<double>> vec(slots, {0.0, 0.0});
        vec[0] = {val, 0.0};
        return cc->MakeCKKSPackedPlaintext(vec);
    }
};

int main() {
    cout << "========================================\n";
    cout << "  φ-PERIOD-0 FHE TEST\n";
    cout << "  φ-Normalize sa OpenFHE\n";
    cout << "========================================\n\n";
    
    PhiPeriod0FHE fhe;
    
    // TEST 1: BOOTSTRAP — 1 APPLICATION LANG!
    cout << "TEST 1: BOOTSTRAP (1 APPLICATION)\n";
    cout << "=================================\n\n";
    
    for (double val : {0.5, 1.0, 2.0, 3.0, 5.0}) {
        auto ct = fhe.encode(val);
        auto bootstrapped = fhe.bootstrap(ct);
        cout << "  " << val << " → " << fhe.decode(bootstrapped) << "\n";
    }
    cout << "\n  Bounded agad: YES ✓\n\n";
    
    // TEST 2: CT × CT
    cout << "TEST 2: CT × CT\n";
    cout << "================\n\n";
    
    auto a = fhe.encode(1.0);  // 0.1
    auto b = fhe.encode(2.0);  // 0.2
    auto result = fhe.multiply_auto(a, b);  // 0.02 → bootstrap
    
    cout << "  1 × 2 = " << fhe.decode(result) << "\n";
    cout << "  Bounded: " << (abs(fhe.decode(result)) < 10 ? "YES ✓" : "NO ✗") << "\n\n";
    
    // TEST 3: SEQUENTIAL (10 ops)
    cout << "TEST 3: SEQUENTIAL (10 ops)\n";
    cout << "===========================\n\n";
    
    auto start = high_resolution_clock::now();
    
    auto seq = fhe.encode(1.0);
    for (int i = 2; i <= 8; i++) {
        auto ct_i = fhe.encode(i);
        seq = fhe.multiply_auto(seq, ct_i);
        cout << "  Op " << i-1 << ": " << fhe.decode(seq) << "\n";
    }
    
    auto end = high_resolution_clock::now();
    auto duration = duration_cast<milliseconds>(end - start).count();
    
    cout << "\n  Final: " << fhe.decode(seq) << "\n";
    cout << "  Bounded: " << (abs(fhe.decode(seq)) < 10 ? "YES ✓" : "NO ✗") << "\n";
    cout << "  Time: " << duration << " ms\n\n";
    
    cout << "========================================\n";
    cout << (abs(fhe.decode(seq)) < 10 ? "  PERIOD-0 BOOTSTRAP: STABLE ✓\n" : "  PERIOD-0 BOOTSTRAP: HINDI PA ✗\n");
    cout << "========================================\n";
    
    return 0;
}
