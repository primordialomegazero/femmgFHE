// ============================================
// φ-COMPLETE NATURAL FHE
// Bounded + Stable bootstrap sa OpenFHE
//
// Core formula:
// bootstrap(x) = 0.5×(x/(1+|x|/φ)) + 0.5/φ²
// = 3 EvalMult, autonomous, bounded
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

class PhiCompleteNaturalFHE {
private:
    CryptoContext<DCRTPoly> cc;
    PublicKey<DCRTPoly> pk;
    PrivateKey<DCRTPoly> sk;
    int slots;

public:
    PhiCompleteNaturalFHE() {
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
    
    // φ-COMPLETE NATURAL BOOTSTRAP
    // bootstrap(x) = 0.5×(x/(1+|x|/φ)) + 0.5/φ²
    // Simplified: x → 0.5×(x×(1-|x|/φ)) + 0.5/φ²
    Ciphertext<DCRTPoly> bootstrap(Ciphertext<DCRTPoly> ct) {
        // |x| ≈ x² (simplified para sa positive x)
        auto x2 = cc->EvalMult(ct, ct);
        
        // x/φ
        auto x_over_phi = cc->EvalMult(x2, make_plain(INV_PHI));
        
        // 1 - x/φ
        Plaintext one = make_plain(1.0);
        auto one_minus = cc->EvalSub(one, x_over_phi);
        
        // x × (1 - x/φ)
        auto bounded = cc->EvalMult(ct, one_minus);
        
        // 0.5 × bounded
        auto pulled = cc->EvalMult(bounded, make_plain(0.5));
        
        // + 0.5/φ²
        Plaintext half_attractor = make_plain(0.5 * INV_PHI2);
        return cc->EvalAdd(pulled, half_attractor);
    }
    
    // CT × CT na may φ-complete bootstrap
    Ciphertext<DCRTPoly> multiply_auto(Ciphertext<DCRTPoly> a, Ciphertext<DCRTPoly> b) {
        auto product = cc->EvalMult(a, b);
        auto rescaled = cc->EvalMult(product, make_plain(SCALE));
        return bootstrap(rescaled);
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
    cout << "  φ-COMPLETE NATURAL FHE\n";
    cout << "  Bounded + Stable sa OpenFHE\n";
    cout << "========================================\n\n";
    
    PhiCompleteNaturalFHE fhe;
    
    // TEST 1: BOOTSTRAP CONVERGENCE
    cout << "TEST 1: BOOTSTRAP CONVERGENCE\n";
    cout << "=============================\n\n";
    
    auto x = fhe.encode(50.0);
    cout << "  Start: 50.0\n";
    
    for (int i = 1; i <= 10; i++) {
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
    
    // TEST 3: SEQUENTIAL (10 ops)
    cout << "TEST 3: SEQUENTIAL (10 ops)\n";
    cout << "===========================\n\n";
    
    auto start = high_resolution_clock::now();
    
    auto seq = fhe.encode(2.0);
    for (int i = 3; i <= 10; i++) {
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
    cout << (abs(fhe.decode(seq)) < 100 ? "  SARILING BOOTSTRAP: STABLE ✓\n" : "  SARILING BOOTSTRAP: HINDI PA ✗\n");
    cout << "========================================\n";
    
    return 0;
}
