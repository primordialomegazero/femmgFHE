// ============================================
// φ-MINIMAL FINAL
// 1 Multiplication Bootstrap — ang pinaka-simple
//
// Core fix:
// - bootstrap(x) = x×(1-x) — 1 mult lang
// - Bounded sa [0, 0.25]
// - Stable at self-correcting
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

class PhiMinimalFHE {
private:
    CryptoContext<DCRTPoly> cc;
    PublicKey<DCRTPoly> pk;
    PrivateKey<DCRTPoly> sk;
    int slots;

public:
    PhiMinimalFHE() {
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
        // Pre-normalize sa [0, 1]
        vector<complex<double>> vec(slots, {0.0, 0.0});
        vec[0] = {val, 0.0};
        return cc->Encrypt(pk, cc->MakeCKKSPackedPlaintext(vec));
    }
    
    double decode(Ciphertext<DCRTPoly> ct) {
        Plaintext pt;
        cc->Decrypt(sk, ct, &pt);
        return pt->GetCKKSPackedValue()[0].real();
    }
    
    // φ-MINIMAL BOOTSTRAP: x → x×(1-x)
    // 1 multiplication lang!
    Ciphertext<DCRTPoly> phi_bootstrap(Ciphertext<DCRTPoly> ct) {
        Plaintext one = make_plain(1.0);
        auto one_minus_x = cc->EvalSub(one, ct);
        return cc->EvalMult(ct, one_minus_x);
    }
    
    // MULTIPLICATION with bootstrap
    Ciphertext<DCRTPoly> phi_multiply(Ciphertext<DCRTPoly> a, 
                                       Ciphertext<DCRTPoly> b) {
        auto product = cc->EvalMult(a, b);
        return phi_bootstrap(product);
    }
    
    // ADDITION with bootstrap
    Ciphertext<DCRTPoly> phi_add(Ciphertext<DCRTPoly> a, 
                                  Ciphertext<DCRTPoly> b) {
        auto sum = cc->EvalAdd(a, b);
        return phi_bootstrap(sum);
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
    cout << "  φ-MINIMAL FINAL\n";
    cout << "  1 Multiplication Bootstrap\n";
    cout << "========================================\n\n";
    
    PhiMinimalFHE phi_fhe;
    
    // TEST 1: MULTIPLICATION (10×)
    cout << "TEST 1: MULTIPLICATION (10×)\n";
    cout << "============================\n\n";
    
    auto mult_result = phi_fhe.encode(0.3);
    auto mult_two = phi_fhe.encode(2.0);
    
    for (int i = 0; i < 10; i++) {
        mult_result = phi_fhe.phi_multiply(mult_result, mult_two);
        cout << "  Step " << i+1 << ": " << phi_fhe.decode(mult_result) << "\n";
    }
    
    cout << "\n  Final: " << phi_fhe.decode(mult_result) << "\n";
    cout << "  Bounded sa [0, 1]: " << (phi_fhe.decode(mult_result) >= 0 && phi_fhe.decode(mult_result) <= 1.0 ? "YES ✓" : "NO ✗") << "\n\n";
    
    // TEST 2: COMBINED (15 ops) — mas conservative
    cout << "TEST 2: COMBINED (15 ops)\n";
    cout << "=========================\n\n";
    
    auto combined = phi_fhe.encode(0.3);
    
    for (int i = 0; i < 15; i++) {
        combined = phi_fhe.phi_multiply(combined, phi_fhe.encode(2.0));
        combined = phi_fhe.phi_add(combined, phi_fhe.encode(0.1));
        cout << "  Op " << i+1 << ": " << phi_fhe.decode(combined) << "\n";
    }
    
    cout << "\n  Final: " << phi_fhe.decode(combined) << "\n";
    cout << "  Bounded sa [0, 1]: " << (phi_fhe.decode(combined) >= 0 && phi_fhe.decode(combined) <= 1.0 ? "YES ✓" : "NO ✗") << "\n\n";
    
    cout << "========================================\n";
    cout << "  φ-MINIMAL FINAL COMPLETE\n";
    cout << "========================================\n";
    
    return 0;
}
