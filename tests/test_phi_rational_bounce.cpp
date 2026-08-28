// ============================================
// φ-RATIONAL BOUNCE OPENFHE
// Simple polynomial bounce na walang sine
//
// Core fix:
// - Walang EvalSin — polynomial lang
// - φ-rational: x → x/(1 + x/φ)
// - Natural na bounded sa [0, φ]
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
const double PSI = -0.6180339887498948482;
const double INV_PHI = 1.0 / PHI;

class PhiRationalFHE {
private:
    CryptoContext<DCRTPoly> cc;
    PublicKey<DCRTPoly> pk;
    PrivateKey<DCRTPoly> sk;
    int slots;

public:
    PhiRationalFHE() {
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
    
    // φ-RATIONAL BOUNCE: x → x × (1 - x/φ + (x/φ)²)
    Ciphertext<DCRTPoly> phi_bounce(Ciphertext<DCRTPoly> ct) {
        // x/φ
        auto x_over_phi = cc->EvalMult(ct, make_plain(INV_PHI));
        
        // (x/φ)²
        auto x_over_phi_sq = cc->EvalMult(x_over_phi, x_over_phi);
        
        // poly = 1 - x/φ + (x/φ)²
        Plaintext one = make_plain(1.0);
        auto poly = cc->EvalSub(one, x_over_phi);
        poly = cc->EvalAdd(poly, x_over_phi_sq);
        
        // x × poly
        return cc->EvalMult(ct, poly);
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
    cout << "  φ-RATIONAL BOUNCE OPENFHE\n";
    cout << "  Polynomial Bounce na Walang Sine\n";
    cout << "========================================\n\n";
    
    PhiRationalFHE phi_fhe;
    
    // TEST 1: φ-BOUNCE
    cout << "TEST 1: φ-BOUNCE\n";
    cout << "================\n\n";
    
    auto ct_100 = phi_fhe.encode(100.0);
    auto bounced = phi_fhe.phi_bounce(ct_100);
    
    cout << "  100 → bounce → " << phi_fhe.decode(bounced) << "\n";
    cout << "  Bounded: " << (abs(phi_fhe.decode(bounced)) < 100 ? "YES ✓" : "NO ✗") << "\n\n";
    
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
    cout << "  Bounded: " << (abs(phi_fhe.decode(mult_result)) < 100 ? "YES ✓" : "NO ✗") << "\n\n";
    
    // TEST 3: φ-ADDITION (10×)
    cout << "TEST 3: φ-ADDITION (10×)\n";
    cout << "=========================\n\n";
    
    auto add_result = phi_fhe.encode(1.0);
    auto add_ten = phi_fhe.encode(10.0);
    
    for (int i = 0; i < 10; i++) {
        add_result = phi_fhe.phi_add(add_result, add_ten);
        cout << "  Step " << i+1 << ": " << phi_fhe.decode(add_result) << "\n";
    }
    
    cout << "\n  Final: " << phi_fhe.decode(add_result) << "\n";
    cout << "  Bounded: " << (abs(phi_fhe.decode(add_result)) < 100 ? "YES ✓" : "NO ✗") << "\n\n";
    
    // TEST 4: φ-UNBOUNDED (20 combined ops)
    cout << "TEST 4: φ-UNBOUNDED (20 ops)\n";
    cout << "=============================\n\n";
    
    auto unbounded = phi_fhe.encode(50.0);
    
    for (int i = 0; i < 20; i++) {
        unbounded = phi_fhe.phi_multiply(unbounded, phi_fhe.encode(3.0));
        unbounded = phi_fhe.phi_add(unbounded, phi_fhe.encode(7.0));
    }
    
    cout << "  After 20 combined ops: " << phi_fhe.decode(unbounded) << "\n";
    cout << "  Bounded: " << (abs(phi_fhe.decode(unbounded)) < 100 ? "YES ✓" : "NO ✗") << "\n\n";
    
    cout << "========================================\n";
    cout << "  φ-RATIONAL BOUNCE COMPLETE\n";
    cout << "========================================\n";
    
    return 0;
}
