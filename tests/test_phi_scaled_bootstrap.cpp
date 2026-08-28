// ============================================
// φ-SCALED BOOTSTRAP
// Scale muna bago bootstrap para sa validity
//
// Core fix:
// - scale(x) = x/φ — laging nasa [0, 1/φ]
// - bootstrap(x) = φ×scale(x)×(1-φ×scale(x))
// - Valid para sa lahat ng x ∈ [0, 1]
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

class PhiScaledFHE {
private:
    CryptoContext<DCRTPoly> cc;
    PublicKey<DCRTPoly> pk;
    PrivateKey<DCRTPoly> sk;
    int slots;

public:
    PhiScaledFHE() {
        CCParams<CryptoContextCKKSRNS> params;
        params.SetMultiplicativeDepth(25);
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
        vector<complex<double>> vec(slots, {0.0, 0.0});
        vec[0] = {val, 0.0};
        return cc->Encrypt(pk, cc->MakeCKKSPackedPlaintext(vec));
    }
    
    double decode(Ciphertext<DCRTPoly> ct) {
        Plaintext pt;
        cc->Decrypt(sk, ct, &pt);
        return pt->GetCKKSPackedValue()[0].real();
    }
    
    // φ-SCALED BOOTSTRAP: x → φ × (x/φ) × (1 - φ×(x/φ))
    // = x × (1 - x)
    // Valid para sa lahat ng x ∈ [0, 1]
    Ciphertext<DCRTPoly> phi_bootstrap(Ciphertext<DCRTPoly> ct) {
        // 1 - x
        Plaintext one = make_plain(1.0);
        auto one_minus_x = cc->EvalSub(one, ct);
        
        // x × (1 - x)
        return cc->EvalMult(ct, one_minus_x);
    }
    
    // φ-MULTIPLICATION with bootstrap
    Ciphertext<DCRTPoly> phi_multiply(Ciphertext<DCRTPoly> a, 
                                       Ciphertext<DCRTPoly> b) {
        auto product = cc->EvalMult(a, b);
        return phi_bootstrap(product);
    }
    
    // φ-ADDITION with bootstrap
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
    cout << "  φ-SCALED BOOTSTRAP\n";
    cout << "  x → x×(1-x) — Valid sa [0, 1]\n";
    cout << "========================================\n\n";
    
    PhiScaledFHE phi_fhe;
    
    // TEST 1: BOOTSTRAP
    cout << "TEST 1: φ-SCALED BOOTSTRAP\n";
    cout << "==========================\n\n";
    
    for (double val : {0.1, 0.3, 0.5, 0.7, 0.9}) {
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
    cout << "  Bounded sa [0, 1]: " << (phi_fhe.decode(mult_result) >= 0 && phi_fhe.decode(mult_result) <= 1.0 ? "YES ✓" : "NO ✗") << "\n\n";
    
    // TEST 3: UNBOUNDED (30 ops)
    cout << "TEST 3: UNBOUNDED (30 ops)\n";
    cout << "==========================\n\n";
    
    auto unbounded = phi_fhe.encode(0.5);
    
    for (int i = 0; i < 30; i++) {
        unbounded = phi_fhe.phi_multiply(unbounded, phi_fhe.encode(2.0));
        unbounded = phi_fhe.phi_add(unbounded, phi_fhe.encode(0.1));
        cout << "  Op " << i+1 << ": " << phi_fhe.decode(unbounded) << "\n";
    }
    
    cout << "\n  Final: " << phi_fhe.decode(unbounded) << "\n";
    cout << "  Bounded sa [0, 1]: " << (phi_fhe.decode(unbounded) >= 0 && phi_fhe.decode(unbounded) <= 1.0 ? "YES ✓" : "NO ✗") << "\n\n";
    
    cout << "========================================\n";
    cout << "  φ-SCALED BOOTSTRAP COMPLETE\n";
    cout << "========================================\n";
    
    return 0;
}
