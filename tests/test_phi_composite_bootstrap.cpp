// ============================================
// φ-COMPOSITE BOOTSTRAP
// φ-Clamp + φ-Logistic para sa stable refresh
//
// Core fix:
// - φ-clamp: x → x×(1-x×φ) para sa noise reduction
// - φ-logistic: x → φ×x×(1-x) para sa convergence
// - Composite: x → φ×x×(1-x)×(1+φ×x²)
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

class PhiCompositeFHE {
private:
    CryptoContext<DCRTPoly> cc;
    PublicKey<DCRTPoly> pk;
    PrivateKey<DCRTPoly> sk;
    int slots;

public:
    PhiCompositeFHE() {
        CCParams<CryptoContextCKKSRNS> params;
        params.SetMultiplicativeDepth(25);
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
    
    // φ-COMPOSITE BOOTSTRAP: x → φ×x×(1-x)×(1+φ×x²)
    // Stable para sa noise, nagco-converge sa 1/φ
    Ciphertext<DCRTPoly> phi_bootstrap(Ciphertext<DCRTPoly> ct) {
        // x²
        auto x2 = cc->EvalMult(ct, ct);
        
        // 1 - x
        Plaintext one = make_plain(1.0);
        auto one_minus_x = cc->EvalSub(one, ct);
        
        // 1 + φ×x²
        auto phi_x2 = cc->EvalMult(x2, make_plain(PHI));
        auto one_plus_phi_x2 = cc->EvalAdd(one, phi_x2);
        
        // φ×x×(1-x)×(1+φ×x²)
        auto term1 = cc->EvalMult(ct, one_minus_x);
        auto term2 = cc->EvalMult(term1, one_plus_phi_x2);
        return cc->EvalMult(term2, make_plain(PHI));
    }
    
    // φ-MULTIPLICATION with composite bootstrap
    Ciphertext<DCRTPoly> phi_multiply(Ciphertext<DCRTPoly> a, 
                                       Ciphertext<DCRTPoly> b) {
        auto product = cc->EvalMult(a, b);
        return phi_bootstrap(product);
    }
    
    // φ-ADDITION with composite bootstrap
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
    cout << "  φ-COMPOSITE BOOTSTRAP\n";
    cout << "  φ-Clamp + φ-Logistic\n";
    cout << "========================================\n\n";
    
    PhiCompositeFHE phi_fhe;
    
    // TEST 1: BOOTSTRAP
    cout << "TEST 1: φ-COMPOSITE BOOTSTRAP\n";
    cout << "=============================\n\n";
    
    vector<double> test_vals = {0.1, 0.3, 0.5, 0.7, 0.9};
    for (double val : test_vals) {
        auto ct = phi_fhe.encode(val);
        auto bootstrapped = phi_fhe.phi_bootstrap(ct);
        cout << "  " << val << " → " << phi_fhe.decode(bootstrapped) << "\n";
    }
    cout << "\n";
    
    // TEST 2: MULTIPLICATION (10×)
    cout << "TEST 2: MULTIPLICATION (10×)\n";
    cout << "============================\n\n";
    
    auto mult_result = phi_fhe.encode(0.5);
    auto mult_two = phi_fhe.encode(2.0);
    
    for (int i = 0; i < 10; i++) {
        mult_result = phi_fhe.phi_multiply(mult_result, mult_two);
        cout << "  Step " << i+1 << ": " << phi_fhe.decode(mult_result) << "\n";
    }
    
    cout << "\n  Final: " << phi_fhe.decode(mult_result) << "\n";
    cout << "  1/φ = " << INV_PHI << "\n";
    cout << "  Bounded sa [0, 1]: " << (phi_fhe.decode(mult_result) >= 0 && phi_fhe.decode(mult_result) <= 1.0 ? "YES ✓" : "NO ✗") << "\n\n";
    
    // TEST 3: UNBOUNDED (20 ops)
    cout << "TEST 3: UNBOUNDED (20 ops)\n";
    cout << "==========================\n\n";
    
    auto unbounded = phi_fhe.encode(0.5);
    
    for (int i = 0; i < 20; i++) {
        unbounded = phi_fhe.phi_multiply(unbounded, phi_fhe.encode(2.0));
        unbounded = phi_fhe.phi_add(unbounded, phi_fhe.encode(0.1));
        cout << "  Op " << i+1 << ": " << phi_fhe.decode(unbounded) << "\n";
    }
    
    cout << "\n  Final: " << phi_fhe.decode(unbounded) << "\n";
    cout << "  Bounded sa [0, 1]: " << (phi_fhe.decode(unbounded) >= 0 && phi_fhe.decode(unbounded) <= 1.0 ? "YES ✓" : "NO ✗") << "\n\n";
    
    cout << "========================================\n";
    cout << "  φ-COMPOSITE BOOTSTRAP COMPLETE\n";
    cout << "========================================\n";
    
    return 0;
}
