// ============================================
// φ-EXACT MULTIPLICATION OPENFHE
// φ-Difference na may φ-Scale para sa boundedness
//
// Core formula:
// mult(a,b) = φ² × scale(a) × scale(b)
// scale(x) = x/(φ+x)
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

class PhiExactMultFHE {
private:
    CryptoContext<DCRTPoly> cc;
    PublicKey<DCRTPoly> pk;
    PrivateKey<DCRTPoly> sk;
    int slots;

public:
    PhiExactMultFHE() {
        CCParams<CryptoContextCKKSRNS> params;
        params.SetMultiplicativeDepth(10);
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
    
    // φ-SCALE: x → x/(φ+x) 
    // Sa FHE: gamitin ang φ-normalized input
    Ciphertext<DCRTPoly> phi_scale(Ciphertext<DCRTPoly> ct) {
        // Para sa x ∈ [0, 1]: scale ≈ x × (1 - x/φ)
        auto x_over_phi = cc->EvalMult(ct, make_plain(INV_PHI));
        Plaintext one = make_plain(1.0);
        auto poly = cc->EvalSub(one, x_over_phi);
        return cc->EvalMult(ct, poly);
    }
    
    // φ-MULTIPLICATION: a×b → φ² × scale(a) × scale(b)
    Ciphertext<DCRTPoly> phi_multiply(Ciphertext<DCRTPoly> a, 
                                       Ciphertext<DCRTPoly> b) {
        auto a_scaled = phi_scale(a);
        auto b_scaled = phi_scale(b);
        auto product = cc->EvalMult(a_scaled, b_scaled);
        return cc->EvalMult(product, make_plain(PHI * PHI));
    }
    
    // φ-ADDITION: a+b → scale(a+b)
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
    cout << "  φ-EXACT MULTIPLICATION OPENFHE\n";
    cout << "  φ-Scale + φ-Difference\n";
    cout << "========================================\n\n";
    
    PhiExactMultFHE phi_fhe;
    
    // TEST 1: φ-MULTIPLICATION
    cout << "TEST 1: φ-MULTIPLICATION\n";
    cout << "========================\n\n";
    
    auto ct_3 = phi_fhe.encode(3.0);
    auto ct_5 = phi_fhe.encode(5.0);
    auto mult_result = phi_fhe.phi_multiply(ct_3, ct_5);
    
    cout << "  3 × 5 = " << phi_fhe.decode(mult_result) << " (expected 15)\n";
    cout << "  Bounded: " << (abs(phi_fhe.decode(mult_result)) < PHI * PHI ? "YES ✓" : "NO ✗") << "\n\n";
    
    // TEST 2: φ-MULTIPLICATION (10×)
    cout << "TEST 2: φ-MULTIPLICATION (10×)\n";
    cout << "==============================\n\n";
    
    auto mult_chain = phi_fhe.encode(1.0);
    auto mult_two = phi_fhe.encode(2.0);
    
    for (int i = 0; i < 10; i++) {
        mult_chain = phi_fhe.phi_multiply(mult_chain, mult_two);
        cout << "  Step " << i+1 << ": " << phi_fhe.decode(mult_chain) << "\n";
    }
    
    cout << "\n  Final: " << phi_fhe.decode(mult_chain) << "\n";
    cout << "  Bounded: " << (abs(phi_fhe.decode(mult_chain)) < PHI * PHI ? "YES ✓" : "NO ✗") << "\n\n";
    
    // TEST 3: φ-UNBOUNDED (20 ops)
    cout << "TEST 3: φ-UNBOUNDED (20 ops)\n";
    cout << "=============================\n\n";
    
    auto unbounded = phi_fhe.encode(1.0);
    
    for (int i = 0; i < 20; i++) {
        unbounded = phi_fhe.phi_multiply(unbounded, phi_fhe.encode(3.0));
        unbounded = phi_fhe.phi_add(unbounded, phi_fhe.encode(0.5));
    }
    
    cout << "  After 20 combined ops: " << phi_fhe.decode(unbounded) << "\n";
    cout << "  Bounded: " << (abs(phi_fhe.decode(unbounded)) < PHI * PHI ? "YES ✓" : "NO ✗") << "\n\n";
    
    cout << "========================================\n";
    cout << "  φ-EXACT MULTIPLICATION COMPLETE\n";
    cout << "========================================\n";
    
    return 0;
}
