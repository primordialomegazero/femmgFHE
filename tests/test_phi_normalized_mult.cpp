// ============================================
// φ-NORMALIZED MULTIPLICATION
// Lahat ng data sa [0, 1] — walang scale needed
//
// Core fix:
// - I-encode ang lahat sa [0, 1]
// - Multiplication ay natural na bounded
// - Walang polynomial approximation
//
// Author: Dan Fernandez / Primordial Omega Zero
// ============================================

#include "openfhe.h"
#include <iostream>
#include <vector>
#include <cmath>
#include <algorithm>

using namespace lbcrypto;
using namespace std;

const double PHI = 1.6180339887498948482;
const double INV_PHI = 1.0 / PHI;

class PhiNormalizedFHE {
private:
    CryptoContext<DCRTPoly> cc;
    PublicKey<DCRTPoly> pk;
    PrivateKey<DCRTPoly> sk;
    int slots;

public:
    PhiNormalizedFHE() {
        CCParams<CryptoContextCKKSRNS> params;
        params.SetMultiplicativeDepth(20);
        params.SetScalingModSize(50);
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
    
    // ENCODE: i-normalize sa [0, 1]
    Ciphertext<DCRTPoly> encode(double val) {
        // φ-normalization: val → val/(φ+val) — laging sa [0, 1]
        double normalized = val / (PHI + val);
        vector<complex<double>> vec(slots, {0.0, 0.0});
        vec[0] = {normalized, 0.0};
        return cc->Encrypt(pk, cc->MakeCKKSPackedPlaintext(vec));
    }
    
    double decode(Ciphertext<DCRTPoly> ct) {
        Plaintext pt;
        cc->Decrypt(sk, ct, &pt);
        double normalized = pt->GetCKKSPackedValue()[0].real();
        // φ-decode: normalized → normalized×φ/(1-normalized)
        if (normalized >= 1.0) return 999999.0;
        return normalized * PHI / (1.0 - normalized);
    }
    
    // MULTIPLICATION: natural na bounded sa [0, 1]
    Ciphertext<DCRTPoly> phi_multiply(Ciphertext<DCRTPoly> a, 
                                       Ciphertext<DCRTPoly> b) {
        return cc->EvalMult(a, b);
    }
    
    // ADDITION: natural na bounded sa [0, 1] (approx)
    Ciphertext<DCRTPoly> phi_add(Ciphertext<DCRTPoly> a, 
                                  Ciphertext<DCRTPoly> b) {
        return cc->EvalAdd(a, b);
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
    cout << "  φ-NORMALIZED MULTIPLICATION\n";
    cout << "  Lahat sa [0, 1] — Walang Scale\n";
    cout << "========================================\n\n";
    
    PhiNormalizedFHE phi_fhe;
    
    // TEST 1: ENCODING
    cout << "TEST 1: φ-NORMALIZED ENCODING\n";
    cout << "=============================\n\n";
    
    vector<double> test_vals = {0.5, 1.0, 3.0, 5.0, 10.0, 100.0};
    for (double val : test_vals) {
        auto ct = phi_fhe.encode(val);
        double decoded = phi_fhe.decode(ct);
        cout << "  " << val << " → encode → decode → " << decoded << "\n";
    }
    cout << "\n";
    
    // TEST 2: MULTIPLICATION (10×)
    cout << "TEST 2: MULTIPLICATION (10×)\n";
    cout << "============================\n\n";
    
    auto mult_result = phi_fhe.encode(1.0);
    auto mult_two = phi_fhe.encode(2.0);
    
    for (int i = 0; i < 10; i++) {
        mult_result = phi_fhe.phi_multiply(mult_result, mult_two);
        cout << "  Step " << i+1 << ": " << phi_fhe.decode(mult_result) << "\n";
    }
    
    cout << "\n  Final: " << phi_fhe.decode(mult_result) << "\n";
    cout << "  Bounded sa [0, φ]: " << (phi_fhe.decode(mult_result) >= 0 && phi_fhe.decode(mult_result) < PHI ? "YES ✓" : "NO ✗") << "\n\n";
    
    // TEST 3: UNBOUNDED (20 ops)
    cout << "TEST 3: UNBOUNDED (20 ops)\n";
    cout << "===========================\n\n";
    
    auto unbounded = phi_fhe.encode(1.0);
    
    for (int i = 0; i < 20; i++) {
        unbounded = phi_fhe.phi_multiply(unbounded, phi_fhe.encode(3.0));
        unbounded = phi_fhe.phi_add(unbounded, phi_fhe.encode(0.5));
    }
    
    cout << "  After 20 combined ops: " << phi_fhe.decode(unbounded) << "\n";
    cout << "  Bounded sa [0, φ]: " << (phi_fhe.decode(unbounded) >= 0 && phi_fhe.decode(unbounded) < PHI ? "YES ✓" : "NO ✗") << "\n\n";
    
    cout << "========================================\n";
    cout << "  φ-NORMALIZED MULTIPLICATION COMPLETE\n";
    cout << "========================================\n";
    
    return 0;
}
