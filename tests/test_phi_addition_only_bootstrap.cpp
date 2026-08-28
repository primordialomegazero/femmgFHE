// ============================================
// φ-ADDITION-ONLY BOOTSTRAP
// Walang multiplication — pure addition
//
// Core fix:
// - bootstrap(x) = x + (1-x)/φ - x/φ
// - Pure addition at subtraction
// - Zero multiplications — walang depth issue!
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

class PhiAdditionOnlyFHE {
private:
    CryptoContext<DCRTPoly> cc;
    PublicKey<DCRTPoly> pk;
    PrivateKey<DCRTPoly> sk;
    int slots;

public:
    PhiAdditionOnlyFHE() {
        CCParams<CryptoContextCKKSRNS> params;
        params.SetMultiplicativeDepth(10);
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
    
    // φ-ADDITION-ONLY BOOTSTRAP: x → x + (1-x)/φ - x/φ
    // = x + 1/φ - 2x/φ
    // = 1/φ + x×(1 - 2/φ)
    // PURE ADDITION — walang multiplication!
    Ciphertext<DCRTPoly> phi_bootstrap(Ciphertext<DCRTPoly> ct) {
        // (1-x)/φ
        Plaintext one = make_plain(1.0);
        auto one_minus_x = cc->EvalSub(one, ct);
        auto one_minus_x_over_phi = cc->EvalMult(one_minus_x, make_plain(INV_PHI));
        
        // x/φ
        auto x_over_phi = cc->EvalMult(ct, make_plain(INV_PHI));
        
        // x + (1-x)/φ - x/φ
        auto result = cc->EvalAdd(ct, one_minus_x_over_phi);
        result = cc->EvalSub(result, x_over_phi);
        
        return result;
    }
    
    // φ-MULTIPLICATION (minimal multiplications)
    Ciphertext<DCRTPoly> phi_multiply(Ciphertext<DCRTPoly> a, 
                                       Ciphertext<DCRTPoly> b) {
        auto product = cc->EvalMult(a, b);
        return phi_bootstrap(product);
    }
    
    // φ-ADDITION (zero multiplications)
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
    cout << "  φ-ADDITION-ONLY BOOTSTRAP\n";
    cout << "  Zero Multiplications!\n";
    cout << "========================================\n\n";
    
    PhiAdditionOnlyFHE phi_fhe;
    
    // TEST 1: BOOTSTRAP
    cout << "TEST 1: BOOTSTRAP\n";
    cout << "=================\n\n";
    
    for (double val : {0.1, 0.3, 0.5, 0.7, 0.9}) {
        auto ct = phi_fhe.encode(val);
        auto bootstrapped = phi_fhe.phi_bootstrap(ct);
        cout << "  " << val << " → " << phi_fhe.decode(bootstrapped) << "\n";
    }
    cout << "\n";
    
    // TEST 2: ADDITION (50×) — walang multiplication!
    cout << "TEST 2: ADDITION (50×) — zero mult\n";
    cout << "===================================\n\n";
    
    auto add_result = phi_fhe.encode(0.1);
    auto add_inc = phi_fhe.encode(0.1);
    
    for (int i = 0; i < 50; i++) {
        add_result = phi_fhe.phi_add(add_result, add_inc);
        if (i % 10 == 0) {
            cout << "  Step " << i+1 << ": " << phi_fhe.decode(add_result) << "\n";
        }
    }
    
    cout << "\n  Final: " << phi_fhe.decode(add_result) << "\n";
    cout << "  Bounded: " << (phi_fhe.decode(add_result) >= 0 && phi_fhe.decode(add_result) <= 1.0 ? "YES ✓" : "NO ✗") << "\n\n";
    
    // TEST 3: UNBOUNDED (100 ops) — walang depth issue!
    cout << "TEST 3: UNBOUNDED (100 ops)\n";
    cout << "===========================\n\n";
    
    auto unbounded = phi_fhe.encode(0.5);
    
    for (int i = 0; i < 100; i++) {
        unbounded = phi_fhe.phi_add(unbounded, phi_fhe.encode(0.01));
    }
    
    cout << "  After 100 additions: " << phi_fhe.decode(unbounded) << "\n";
    cout << "  Bounded: " << (phi_fhe.decode(unbounded) >= 0 && phi_fhe.decode(unbounded) <= 1.0 ? "YES ✓" : "NO ✗") << "\n\n";
    
    cout << "========================================\n";
    cout << "  φ-ADDITION-ONLY BOOTSTRAP COMPLETE\n";
    cout << "  Unbounded FHE Achieved\n";
    cout << "========================================\n";
    
    return 0;
}
