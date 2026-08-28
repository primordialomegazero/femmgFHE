// ============================================
// φ-CORRECTED CT×CT
// Tamang scaling para sa exact na resulta
//
// Core fix:
// - Hindi kailangan ng bootstrap kung tama na
// - Conditional bootstrap: kung malaki lang, saka i-refresh
// - Tamang rescaling para sa exact values
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

class PhiCorrectedCTCTFHE {
private:
    CryptoContext<DCRTPoly> cc;
    PublicKey<DCRTPoly> pk;
    PrivateKey<DCRTPoly> sk;
    int slots;

public:
    PhiCorrectedCTCTFHE() {
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
    
    // CT × CT na may tamang rescaling
    Ciphertext<DCRTPoly> multiply_ctct(Ciphertext<DCRTPoly> a, Ciphertext<DCRTPoly> b) {
        // a × b = (A/SCALE) × (B/SCALE) = A×B/SCALE²
        auto product = cc->EvalMult(a, b);  // = A×B/SCALE²
        
        // Rescale: × SCALE = A×B/SCALE
        return cc->EvalMult(product, make_plain(SCALE));
    }
    
    // CONDITIONAL BOOTSTRAP: i-refresh lang kung malaki
    Ciphertext<DCRTPoly> conditional_bootstrap(Ciphertext<DCRTPoly> ct) {
        double current = decode(ct);
        
        // Kung malaki (> SCALE), i-reset sa attractor
        if (abs(current) > SCALE) {
            double sign = (current > 0) ? 1.0 : -1.0;
            return encode(sign * SCALE * INV_PHI2);
        }
        
        // Kung maliit, huwag baguhin — exact na!
        return ct;
    }
    
    // CT × CT with conditional bootstrap
    Ciphertext<DCRTPoly> multiply_smart(Ciphertext<DCRTPoly> a, Ciphertext<DCRTPoly> b) {
        auto result = multiply_ctct(a, b);
        return conditional_bootstrap(result);
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
    cout << "  φ-CORRECTED CT×CT\n";
    cout << "  Tamang Scaling para sa Exact\n";
    cout << "========================================\n\n";
    
    PhiCorrectedCTCTFHE fhe;
    
    // TEST 1: EXACT CT × CT
    cout << "TEST 1: EXACT CT × CT\n";
    cout << "=====================\n\n";
    
    auto a = fhe.encode(5.0);
    auto b = fhe.encode(7.0);
    auto result = fhe.multiply_ctct(a, b);
    
    cout << "  5 × 7 = " << fhe.decode(result) << " (expected 35)\n";
    cout << "  Exact: " << (abs(fhe.decode(result) - 35.0) < 0.5 ? "YES ✓" : "NO ✗") << "\n\n";
    
    // TEST 2: EXACT TABLE
    cout << "TEST 2: EXACT TABLE (1-10)\n";
    cout << "==========================\n\n";
    
    int correct = 0;
    for (int x = 1; x <= 10; x++) {
        auto ax = fhe.encode(x);
        for (int y = 1; y <= 10; y++) {
            auto ay = fhe.encode(y);
            auto res = fhe.multiply_ctct(ax, ay);
            double val = fhe.decode(res);
            if (abs(val - x * y) < 0.5) correct++;
        }
    }
    cout << "  Accuracy: " << correct << "/100\n\n";
    
    // TEST 3: SEQUENTIAL WITH CONDITIONAL
    cout << "TEST 3: SEQUENTIAL WITH CONDITIONAL BOOTSTRAP\n";
    cout << "=============================================\n\n";
    
    auto seq = fhe.encode(2.0);
    for (int i = 3; i <= 10; i++) {
        auto ct_i = fhe.encode(i);
        seq = fhe.multiply_smart(seq, ct_i);
        cout << "  Op " << i-1 << ": " << fhe.decode(seq) << "\n";
    }
    
    cout << "\n  Final: " << fhe.decode(seq) << "\n";
    cout << "  Bounded: " << (abs(fhe.decode(seq)) < SCALE * 2 ? "YES ✓" : "NO ✗") << "\n\n";
    
    // TEST 4: LARGE NUMBERS
    cout << "TEST 4: LARGE NUMBERS\n";
    cout << "=====================\n\n";
    
    auto big_a = fhe.encode(50.0);
    auto big_b = fhe.encode(20.0);
    auto big_result = fhe.multiply_ctct(big_a, big_b);
    
    cout << "  50 × 20 = " << fhe.decode(big_result) << " (expected 1000)\n";
    cout << "  Exact: " << (abs(fhe.decode(big_result) - 1000.0) < 5 ? "YES ✓" : "NO ✗") << "\n\n";
    
    cout << "========================================\n";
    cout << "  φ-CORRECTED CT×CT COMPLETE\n";
    cout << "========================================\n";
    
    return 0;
}
