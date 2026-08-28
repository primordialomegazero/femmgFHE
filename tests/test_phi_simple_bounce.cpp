// ============================================
// φ-SIMPLE BOUNCE OPENFHE
// Simplest bounce na stable sa FHE
//
// Core insight:
// - φ-modular bounce ay exact at bounded
// - Sa FHE, kailangan ng polynomial approximation
// - φ-quadratic bounce: x → x - φ×x²/4
// - 1 multiplication lang, bounded sa [0, φ]
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

class PhiSimpleFHE {
private:
    CryptoContext<DCRTPoly> cc;
    PublicKey<DCRTPoly> pk;
    PrivateKey<DCRTPoly> sk;
    int slots;

public:
    PhiSimpleFHE() {
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
    
    // φ-SIMPLE BOUNCE: x → x - φ×x²/4
    // 1 multiplication lang, bounded sa [0, φ]
    // Para sa x ∈ [0, 2φ]: x - φ×x²/4 ∈ [0, φ]
    Ciphertext<DCRTPoly> phi_bounce(Ciphertext<DCRTPoly> ct) {
        auto x_sq = cc->EvalMult(ct, ct);
        auto phi_x_sq = cc->EvalMult(x_sq, make_plain(INV_PHI * 0.25));
        return cc->EvalSub(ct, phi_x_sq);
    }
    
    // φ-MULTIPLICATION na may bounce
    Ciphertext<DCRTPoly> phi_multiply(Ciphertext<DCRTPoly> a, 
                                       Ciphertext<DCRTPoly> b) {
        auto product = cc->EvalMult(a, b);
        return phi_bounce(product);
    }
    
    // φ-ADDITION na may bounce
    Ciphertext<DCRTPoly> phi_add(Ciphertext<DCRTPoly> a, 
                                  Ciphertext<DCRTPoly> b) {
        auto sum = cc->EvalAdd(a, b);
        return phi_bounce(sum);
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
    cout << "  φ-SIMPLE BOUNCE OPENFHE\n";
    cout << "  Quadratic Bounce na Stable\n";
    cout << "========================================\n\n";
    
    PhiSimpleFHE phi_fhe;
    
    // TEST 1: φ-BOUNCE
    cout << "TEST 1: φ-BOUNCE\n";
    cout << "================\n\n";
    
    vector<double> test_vals = {0.5, 1.0, 2.0, 5.0, 10.0};
    for (double val : test_vals) {
        auto ct = phi_fhe.encode(val);
        auto bounced = phi_fhe.phi_bounce(ct);
        cout << "  " << val << " → " << phi_fhe.decode(bounced) << "\n";
    }
    
    cout << "\n  Lahat bounded: ";
    bool all_bounded = true;
    for (double val : test_vals) {
        auto ct = phi_fhe.encode(val);
        auto bounced = phi_fhe.phi_bounce(ct);
        double b = phi_fhe.decode(bounced);
        if (b < 0 || b > PHI) all_bounded = false;
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
    cout << "  φ-SIMPLE BOUNCE COMPLETE\n";
    cout << "========================================\n";
    
    return 0;
}
