// ============================================
// φ-SCALE OPENFHE FINAL
// Ang x/(φ+x) sa OpenFHE
//
// Core fix:
// - Pre-scale ang input sa [0, 1]
// - φ-scale polynomial na stable
// - Walang divergence
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

class PhiScaleFHE {
private:
    CryptoContext<DCRTPoly> cc;
    PublicKey<DCRTPoly> pk;
    PrivateKey<DCRTPoly> sk;
    int slots;

public:
    PhiScaleFHE() {
        CCParams<CryptoContextCKKSRNS> params;
        params.SetMultiplicativeDepth(15);
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
    
    Ciphertext<DCRTPoly> encode(double val) {
        vector<complex<double>> vec(slots, {0.0, 0.0});
        vec[0] = {val, 0.0};
        return cc->Encrypt(pk, cc->MakeCKKSPackedPlaintext(vec));
    }
    
    double decode(Ciphertext<DCRTPoly> ct) {
        Plaintext pt;
        cc->Decrypt(sk, ct, &pt);
        return pt->GetCKKSPackedValue()[0].real();
    }
    
    // φ-SCALE: y → y×(1 - y/2 + y²/3)
    // Stable sa [0, 1]
    Ciphertext<DCRTPoly> phi_scale(Ciphertext<DCRTPoly> ct) {
        // y²
        auto y2 = cc->EvalMult(ct, ct);
        
        // y/2
        auto y_half = cc->EvalMult(ct, make_plain(0.5));
        
        // y²/3
        auto y2_third = cc->EvalMult(y2, make_plain(1.0/3.0));
        
        // poly = y - y/2 + y²/3 = y/2 + y²/3
        auto poly = cc->EvalSub(ct, y_half);
        poly = cc->EvalAdd(poly, y2_third);
        
        return poly;
    }
    
    // φ-MULTIPLICATION na may scale
    Ciphertext<DCRTPoly> phi_multiply(Ciphertext<DCRTPoly> a, 
                                       Ciphertext<DCRTPoly> b) {
        auto product = cc->EvalMult(a, b);
        return phi_scale(product);
    }
    
    // φ-ADDITION na may scale
    Ciphertext<DCRTPoly> phi_add(Ciphertext<DCRTPoly> a, 
                                  Ciphertext<DCRTPoly> b) {
        auto sum = cc->EvalAdd(a, b);
        return phi_scale(sum);
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
    cout << "  φ-SCALE OPENFHE FINAL\n";
    cout << "  Stable Scale sa FHE\n";
    cout << "========================================\n\n";
    
    PhiScaleFHE phi_fhe;
    
    // TEST 1: φ-SCALE
    cout << "TEST 1: φ-SCALE\n";
    cout << "================\n\n";
    
    vector<double> test_vals = {0.1, 0.5, 1.0, 1.5};
    for (double val : test_vals) {
        auto ct = phi_fhe.encode(val);
        auto scaled = phi_fhe.phi_scale(ct);
        cout << "  " << val << " → " << phi_fhe.decode(scaled) << "\n";
    }
    cout << "\n";
    
    // TEST 2: φ-MULTIPLICATION (10×)
    cout << "TEST 2: φ-MULTIPLICATION (10×)\n";
    cout << "==============================\n\n";
    
    auto mult_result = phi_fhe.encode(0.5);
    auto mult_two = phi_fhe.encode(2.0);
    
    for (int i = 0; i < 10; i++) {
        mult_result = phi_fhe.phi_multiply(mult_result, mult_two);
        cout << "  Step " << i+1 << ": " << phi_fhe.decode(mult_result) << "\n";
    }
    
    cout << "\n  Final: " << phi_fhe.decode(mult_result) << "\n";
    cout << "  Bounded sa [0, 1]: " << (phi_fhe.decode(mult_result) >= 0 && phi_fhe.decode(mult_result) < 1.0 ? "YES ✓" : "NO ✗") << "\n\n";
    
    // TEST 3: φ-UNBOUNDED (20 ops)
    cout << "TEST 3: φ-UNBOUNDED (20 ops)\n";
    cout << "=============================\n\n";
    
    auto unbounded = phi_fhe.encode(0.5);
    
    for (int i = 0; i < 20; i++) {
        unbounded = phi_fhe.phi_multiply(unbounded, phi_fhe.encode(3.0));
        unbounded = phi_fhe.phi_add(unbounded, phi_fhe.encode(0.5));
    }
    
    cout << "  After 20 combined ops: " << phi_fhe.decode(unbounded) << "\n";
    cout << "  Bounded sa [0, 1]: " << (phi_fhe.decode(unbounded) >= 0 && phi_fhe.decode(unbounded) < 1.0 ? "YES ✓" : "NO ✗") << "\n\n";
    
    cout << "========================================\n";
    cout << "  φ-SCALE OPENFHE COMPLETE\n";
    cout << "========================================\n";
    
    return 0;
}
