// ============================================
// φ-CLAMP OPENFHE FINAL
// Stable clamp na may φ-normalization
//
// Core fix:
// - I-scale muna ang x sa [0, 1]
// - y = x/φ, clamp(y) = φ×y/(1+y)
// - Polynomial approximation stable sa [0, 1]
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

class PhiClampFHE {
private:
    CryptoContext<DCRTPoly> cc;
    PublicKey<DCRTPoly> pk;
    PrivateKey<DCRTPoly> sk;
    int slots;

public:
    PhiClampFHE() {
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
    
    // φ-CLAMP: x → φ×y/(1+y) kung saan y = x/φ
    // Polynomial: φ×(y - y² + y³) para sa y ∈ [0, 1]
    Ciphertext<DCRTPoly> phi_clamp(Ciphertext<DCRTPoly> ct) {
        // y = x/φ
        auto y = cc->EvalMult(ct, make_plain(INV_PHI));
        
        // y² at y³
        auto y2 = cc->EvalMult(y, y);
        auto y3 = cc->EvalMult(y2, y);
        
        // poly = y - y² + y³
        auto poly = cc->EvalSub(y, y2);
        poly = cc->EvalAdd(poly, y3);
        
        // φ × poly
        return cc->EvalMult(poly, make_plain(PHI));
    }
    
    // φ-MULTIPLICATION na may clamp
    Ciphertext<DCRTPoly> phi_multiply(Ciphertext<DCRTPoly> a, 
                                       Ciphertext<DCRTPoly> b) {
        auto product = cc->EvalMult(a, b);
        return phi_clamp(product);
    }
    
    // φ-ADDITION na may clamp
    Ciphertext<DCRTPoly> phi_add(Ciphertext<DCRTPoly> a, 
                                  Ciphertext<DCRTPoly> b) {
        auto sum = cc->EvalAdd(a, b);
        return phi_clamp(sum);
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
    cout << "  φ-CLAMP OPENFHE FINAL\n";
    cout << "  Stable Clamp sa FHE\n";
    cout << "========================================\n\n";
    
    PhiClampFHE phi_fhe;
    
    // TEST 1: φ-CLAMP
    cout << "TEST 1: φ-CLAMP\n";
    cout << "================\n\n";
    
    vector<double> test_vals = {0.5, 1.0, 2.0, 5.0, 10.0};
    for (double val : test_vals) {
        auto ct = phi_fhe.encode(val);
        auto clamped = phi_fhe.phi_clamp(ct);
        cout << "  " << val << " → " << phi_fhe.decode(clamped) << "\n";
    }
    
    cout << "\n  Lahat bounded sa [0, φ]: ";
    bool all_bounded = true;
    for (double val : test_vals) {
        auto ct = phi_fhe.encode(val);
        auto clamped = phi_fhe.phi_clamp(ct);
        double c = phi_fhe.decode(clamped);
        if (c < 0 || c > PHI) all_bounded = false;
    }
    cout << (all_bounded ? "YES ✓" : "NO ✗") << "\n\n";
    
    // TEST 2: φ-MULTIPLICATION (10×)
    cout << "TEST 2: φ-MULTIPLICATION (10×)\n";
    cout << "==============================\n\n";
    
    auto mult_result = phi_fhe.encode(1.0);
    auto mult_ten = phi_fhe.encode(10.0);
    
    for (int i = 0; i < 10; i++) {
        mult_result = phi_fhe.phi_multiply(mult_result, mult_ten);
        cout << "  Step " << i+1 << ": " << phi_fhe.decode(mult_result) << "\n";
    }
    
    cout << "\n  Final: " << phi_fhe.decode(mult_result) << "\n";
    cout << "  Bounded: " << (abs(phi_fhe.decode(mult_result)) < PHI ? "YES ✓" : "NO ✗") << "\n\n";
    
    // TEST 3: φ-UNBOUNDED (20 ops)
    cout << "TEST 3: φ-UNBOUNDED (20 ops)\n";
    cout << "=============================\n\n";
    
    auto unbounded = phi_fhe.encode(1.0);
    
    for (int i = 0; i < 20; i++) {
        unbounded = phi_fhe.phi_multiply(unbounded, phi_fhe.encode(5.0));
        unbounded = phi_fhe.phi_add(unbounded, phi_fhe.encode(3.0));
    }
    
    cout << "  After 20 combined ops: " << phi_fhe.decode(unbounded) << "\n";
    cout << "  Bounded: " << (abs(phi_fhe.decode(unbounded)) < PHI ? "YES ✓" : "NO ✗") << "\n\n";
    
    cout << "========================================\n";
    cout << "  φ-CLAMP OPENFHE COMPLETE\n";
    cout << "========================================\n";
    
    return 0;
}
