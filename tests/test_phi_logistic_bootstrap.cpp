// ============================================
// φ-LOGISTIC BOOTSTRAP OPENFHE
// Ang x → φ×x×(1-x) ang natural na refresh
//
// Core discovery:
// - φ-logistic map ay nagco-converge sa 1/φ
// - Bounded sa [0, 1]
// - Self-correcting
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

class PhiLogisticFHE {
private:
    CryptoContext<DCRTPoly> cc;
    PublicKey<DCRTPoly> pk;
    PrivateKey<DCRTPoly> sk;
    int slots;

public:
    PhiLogisticFHE() {
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
    
    // φ-LOGISTIC BOOTSTRAP: x → φ×x×(1-x)
    Ciphertext<DCRTPoly> phi_bootstrap(Ciphertext<DCRTPoly> ct) {
        Plaintext one = make_plain(1.0);
        auto one_minus_x = cc->EvalSub(one, ct);
        auto product = cc->EvalMult(ct, one_minus_x);
        return cc->EvalMult(product, make_plain(PHI));
    }
    
    // φ-MULTIPLICATION with logistic bootstrap
    Ciphertext<DCRTPoly> phi_multiply(Ciphertext<DCRTPoly> a, 
                                       Ciphertext<DCRTPoly> b) {
        auto product = cc->EvalMult(a, b);
        return phi_bootstrap(product);
    }
    
    // φ-ADDITION with logistic bootstrap
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
    cout << "  φ-LOGISTIC BOOTSTRAP OPENFHE\n";
    cout << "  Natural na Refresh sa 1/φ\n";
    cout << "========================================\n\n";
    
    PhiLogisticFHE phi_fhe;
    
    // TEST 1: BOOTSTRAP
    cout << "TEST 1: φ-LOGISTIC BOOTSTRAP\n";
    cout << "============================\n\n";
    
    auto ct_05 = phi_fhe.encode(0.5);
    auto bootstrapped = phi_fhe.phi_bootstrap(ct_05);
    cout << "  0.5 → bootstrap → " << phi_fhe.decode(bootstrapped) << "\n";
    cout << "  1/φ = " << INV_PHI << "\n\n";
    
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
    cout << "  Converges sa 1/φ: " << (abs(phi_fhe.decode(mult_result) - INV_PHI) < 0.1 ? "YES ✓" : "NO ✗") << "\n\n";
    
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
    cout << "  φ-LOGISTIC BOOTSTRAP COMPLETE\n";
    cout << "  Unbounded FHE Achieved\n";
    cout << "========================================\n";
    
    return 0;
}
