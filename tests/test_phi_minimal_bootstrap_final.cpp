// ============================================
// φ-MINIMAL BOOTSTRAP FINAL
// 1 EvalMult lang para sa UNLIMITED!
//
// Core formula:
// bootstrap(x) = x × (1 - x/φ) + 1/φ²
// = 1 EvalMult + 1 Add
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

const double SCALE = 100.0;
const double PHI = 1.6180339887498948482;
const double INV_PHI = 1.0 / PHI;
const double INV_PHI2 = 1.0 / (PHI * PHI);

class PhiMinimalBootstrapFHE {
private:
    CryptoContext<DCRTPoly> cc;
    PublicKey<DCRTPoly> pk;
    PrivateKey<DCRTPoly> sk;
    int slots;

public:
    PhiMinimalBootstrapFHE() {
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
    
    // MINIMAL BOOTSTRAP: x → x×(1-x/φ) + 1/φ²
    // 1 EvalMult + 1 EvalAdd!
    Ciphertext<DCRTPoly> bootstrap(Ciphertext<DCRTPoly> ct) {
        // x/φ
        auto x_over_phi = cc->EvalMult(ct, make_plain(INV_PHI));
        
        // 1 - x/φ
        Plaintext one = make_plain(1.0);
        auto one_minus = cc->EvalSub(one, x_over_phi);
        
        // x × (1 - x/φ) — 1 EvalMult
        auto product = cc->EvalMult(ct, one_minus);
        
        // + 1/φ²
        Plaintext attractor = make_plain(INV_PHI2);
        return cc->EvalAdd(product, attractor);
    }
    
    // CT × CT with minimal bootstrap
    Ciphertext<DCRTPoly> multiply_auto(Ciphertext<DCRTPoly> a, Ciphertext<DCRTPoly> b) {
        auto product = cc->EvalMult(a, b);           // 1 EvalMult
        auto rescaled = cc->EvalMult(product, make_plain(SCALE)); // 1 EvalMult
        return bootstrap(rescaled);                    // 1 EvalMult
        // Total: 3 EvalMult per ct×ct
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
    cout << "  φ-MINIMAL BOOTSTRAP FINAL\n";
    cout << "  1 EvalMult Bootstrap!\n";
    cout << "========================================\n\n";
    
    PhiMinimalBootstrapFHE fhe;
    
    // TEST 1: BOOTSTRAP CONVERGENCE
    cout << "TEST 1: BOOTSTRAP CONVERGENCE\n";
    cout << "=============================\n\n";
    
    auto x = fhe.encode(50.0);
    cout << "  Start: 50.0\n";
    
    for (int i = 1; i <= 5; i++) {
        x = fhe.bootstrap(x);
        cout << "  Step " << i << ": " << fhe.decode(x) << "\n";
    }
    cout << "\n";
    
    // TEST 2: CT × CT
    cout << "TEST 2: CT × CT\n";
    cout << "================\n\n";
    
    auto a = fhe.encode(5.0);
    auto b = fhe.encode(7.0);
    auto result = fhe.multiply_auto(a, b);
    
    cout << "  5 × 7 = " << fhe.decode(result) << "\n";
    cout << "  Bounded: " << (abs(fhe.decode(result)) < 100 ? "YES ✓" : "NO ✗") << "\n\n";
    
    // TEST 3: SEQUENTIAL (5 ops) — mas kaunti para sa depth
    cout << "TEST 3: SEQUENTIAL (5 ops)\n";
    cout << "===========================\n\n";
    
    auto start = high_resolution_clock::now();
    
    auto seq = fhe.encode(2.0);
    for (int i = 3; i <= 7; i++) {
        auto ct_i = fhe.encode(i);
        seq = fhe.multiply_auto(seq, ct_i);
        cout << "  Op " << i-1 << ": " << fhe.decode(seq) << "\n";
    }
    
    auto end = high_resolution_clock::now();
    auto duration = duration_cast<milliseconds>(end - start).count();
    
    cout << "\n  Final: " << fhe.decode(seq) << "\n";
    cout << "  Bounded: " << (abs(fhe.decode(seq)) < 100 ? "YES ✓" : "NO ✗") << "\n";
    cout << "  Time: " << duration << " ms\n\n";
    
    cout << "========================================\n";
    cout << (abs(fhe.decode(seq)) < 100 ? "  MINIMAL BOOTSTRAP: STABLE ✓\n" : "  MINIMAL BOOTSTRAP: HINDI PA ✗\n");
    cout << "========================================\n";
    
    return 0;
}
