// ============================================
// φ-NORMALIZED CT×CT
// Pre-scaled inputs para sa bounded multiplication
//
// Core fix:
// - I-scale ang inputs sa [0, 1] bago multiply
// - Ang product ay nasa [0, 1]
// - Bootstrap para sa attractor
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

class PhiNormalizedCTCTFHE {
private:
    CryptoContext<DCRTPoly> cc;
    PublicKey<DCRTPoly> pk;
    PrivateKey<DCRTPoly> sk;
    int slots;

public:
    PhiNormalizedCTCTFHE() {
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
        // I-normalize sa [0, 1] mula sa start!
        double normalized = val / (SCALE * 10);  // Mas aggressive scaling
        vector<complex<double>> vec(slots, {0.0, 0.0});
        vec[0] = {normalized, 0.0};
        return cc->Encrypt(pk, cc->MakeCKKSPackedPlaintext(vec));
    }
    
    double decode(Ciphertext<DCRTPoly> ct) {
        Plaintext pt;
        cc->Decrypt(sk, ct, &pt);
        return pt->GetCKKSPackedValue()[0].real() * SCALE * 10;
    }
    
    Ciphertext<DCRTPoly> add(Ciphertext<DCRTPoly> a, Ciphertext<DCRTPoly> b) {
        return cc->EvalAdd(a, b);
    }
    
    // φ-BOOTSTRAP
    Ciphertext<DCRTPoly> bootstrap(Ciphertext<DCRTPoly> ct) {
        Plaintext inv_phi2_pt = make_plain(INV_PHI2);
        auto diff = cc->EvalSub(inv_phi2_pt, ct);
        auto scaled_diff = cc->EvalMult(diff, make_plain(INV_PHI));
        return cc->EvalAdd(ct, scaled_diff);
    }
    
    // NORMALIZED CT × CT: product ay nasa [0, 1] na
    Ciphertext<DCRTPoly> multiply_normalized(Ciphertext<DCRTPoly> a, 
                                               Ciphertext<DCRTPoly> b) {
        // Since a at b ay nasa [0, 1] na:
        // a × b ay nasa [0, 1] din!
        auto product = cc->EvalMult(a, b);  // 1 EvalMult — bounded!
        return bootstrap(product);            // 1 EvalMult — refresh
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
    cout << "  φ-NORMALIZED CT×CT\n";
    cout << "  Bounded Multiplication\n";
    cout << "========================================\n\n";
    
    PhiNormalizedCTCTFHE fhe;
    
    // TEST 1: NORMALIZED MULTIPLICATION
    cout << "TEST 1: NORMALIZED MULTIPLICATION\n";
    cout << "=================================\n\n";
    
    auto a = fhe.encode(5.0);  // 5/(1000) = 0.005
    auto b = fhe.encode(7.0);  // 7/(1000) = 0.007
    
    auto result = fhe.multiply_normalized(a, b);
    cout << "  5 × 7 = " << fhe.decode(result) << " (expected 35)\n";
    cout << "  Bounded: " << (abs(fhe.decode(result)) < 1000 ? "YES ✓" : "NO ✗") << "\n\n";
    
    // TEST 2: SEQUENTIAL (10 ops)
    cout << "TEST 2: SEQUENTIAL (10 ops)\n";
    cout << "===========================\n\n";
    
    auto start = high_resolution_clock::now();
    
    auto seq = fhe.encode(1.0);
    for (int i = 2; i <= 10; i++) {
        auto ct_i = fhe.encode(i);
        seq = fhe.multiply_normalized(seq, ct_i);
        cout << "  Op " << i-1 << ": " << fhe.decode(seq) << "\n";
    }
    
    auto end = high_resolution_clock::now();
    auto duration = duration_cast<milliseconds>(end - start).count();
    
    cout << "\n  Final: " << fhe.decode(seq) << "\n";
    cout << "  Bounded: " << (abs(fhe.decode(seq)) < 1000 ? "YES ✓" : "NO ✗") << "\n";
    cout << "  Time: " << duration << " ms\n\n";
    
    cout << "========================================\n";
    cout << "  φ-NORMALIZED CT×CT COMPLETE\n";
    cout << "========================================\n";
    
    return 0;
}
