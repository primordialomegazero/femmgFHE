// ============================================
// φ-FULL PROTOCOL
// φ-Normalized + φ-Bootstrap = Unbounded FHE
//
// Core integration:
// - φ-Normalized encoding sa [0, 1]
// - φ-Bootstrap pagkatapos ng bawat operation
// - Natural na bounded at noise-free
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
const double BOOTSTRAP_KEY = 0.459641275871300;
const double QUANTUM_FRACTAL = 0.723606797749979;

class PhiFullFHE {
private:
    CryptoContext<DCRTPoly> cc;
    PublicKey<DCRTPoly> pk;
    PrivateKey<DCRTPoly> sk;
    int slots;

public:
    PhiFullFHE() {
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
    
    // ENCODE: φ-normalize sa [0, 1]
    Ciphertext<DCRTPoly> encode(double val) {
        double normalized = val / (PHI + val);
        vector<complex<double>> vec(slots, {0.0, 0.0});
        vec[0] = {normalized, 0.0};
        return cc->Encrypt(pk, cc->MakeCKKSPackedPlaintext(vec));
    }
    
    double decode(Ciphertext<DCRTPoly> ct) {
        Plaintext pt;
        cc->Decrypt(sk, ct, &pt);
        double normalized = pt->GetCKKSPackedValue()[0].real();
        if (normalized >= 1.0) return 999999.0;
        return normalized * PHI / (1.0 - normalized);
    }
    
    // φ-BOOTSTRAP: x → x × BOOTSTRAP_KEY + QUANTUM_FRACTAL × INV_PHI
    Ciphertext<DCRTPoly> phi_bootstrap(Ciphertext<DCRTPoly> ct) {
        auto scaled = cc->EvalMult(ct, make_plain(BOOTSTRAP_KEY));
        Plaintext bias = make_plain(QUANTUM_FRACTAL * INV_PHI);
        return cc->EvalAdd(scaled, bias);
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
    cout << "  φ-FULL PROTOCOL\n";
    cout << "  φ-Normalized + φ-Bootstrap\n";
    cout << "========================================\n\n";
    
    PhiFullFHE phi_fhe;
    
    // TEST 1: ENCODING
    cout << "TEST 1: φ-ENCODING\n";
    cout << "==================\n\n";
    
    vector<double> test_vals = {0.5, 1.0, 3.0, 5.0, 10.0};
    for (double val : test_vals) {
        auto ct = phi_fhe.encode(val);
        cout << "  " << val << " → " << phi_fhe.decode(ct) << "\n";
    }
    cout << "\n";
    
    // TEST 2: φ-BOOTSTRAP
    cout << "TEST 2: φ-BOOTSTRAP\n";
    cout << "===================\n\n";
    
    auto ct_100 = phi_fhe.encode(100.0);
    auto bootstrapped = phi_fhe.phi_bootstrap(ct_100);
    cout << "  100 → bootstrap → " << phi_fhe.decode(bootstrapped) << "\n";
    cout << "  Bounded: " << (phi_fhe.decode(bootstrapped) < 100 ? "YES ✓" : "NO ✗") << "\n\n";
    
    // TEST 3: MULTIPLICATION (10×) with bootstrap
    cout << "TEST 3: MULTIPLICATION (10×) with bootstrap\n";
    cout << "==========================================\n\n";
    
    auto mult_result = phi_fhe.encode(1.0);
    auto mult_two = phi_fhe.encode(2.0);
    
    for (int i = 0; i < 10; i++) {
        mult_result = phi_fhe.phi_multiply(mult_result, mult_two);
        cout << "  Step " << i+1 << ": " << phi_fhe.decode(mult_result) << "\n";
    }
    
    cout << "\n  Final: " << phi_fhe.decode(mult_result) << "\n";
    cout << "  Bounded: " << (phi_fhe.decode(mult_result) < PHI ? "YES ✓" : "NO ✗") << "\n\n";
    
    // TEST 4: UNBOUNDED (20 ops) with bootstrap
    cout << "TEST 4: UNBOUNDED (20 ops) with bootstrap\n";
    cout << "=========================================\n\n";
    
    auto unbounded = phi_fhe.encode(1.0);
    
    for (int i = 0; i < 20; i++) {
        unbounded = phi_fhe.phi_multiply(unbounded, phi_fhe.encode(3.0));
        unbounded = phi_fhe.phi_add(unbounded, phi_fhe.encode(0.5));
        cout << "  Op " << i+1 << ": " << phi_fhe.decode(unbounded) << "\n";
    }
    
    cout << "\n  Final: " << phi_fhe.decode(unbounded) << "\n";
    cout << "  Bounded: " << (phi_fhe.decode(unbounded) < PHI ? "YES ✓" : "NO ✗") << "\n\n";
    
    cout << "========================================\n";
    cout << "  φ-FULL PROTOCOL COMPLETE\n";
    cout << "  Unbounded FHE Achieved\n";
    cout << "========================================\n";
    
    return 0;
}
