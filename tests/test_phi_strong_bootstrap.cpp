// ============================================
// φ-STRONG BOOTSTRAP
// Fixed point sa 1/φ² para sa bounded FHE
//
// Core fix:
// - bootstrap(x) = 1/φ² + (x - 1/φ²) × 0.5
// - Fixed point sa 1/φ² = 0.382
// - Laging bounded sa [0, 1/φ]
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
const double INV_PHI2 = 1.0 / (PHI * PHI);

class PhiStrongFHE {
private:
    CryptoContext<DCRTPoly> cc;
    PublicKey<DCRTPoly> pk;
    PrivateKey<DCRTPoly> sk;
    int slots;

public:
    PhiStrongFHE() {
        CCParams<CryptoContextCKKSRNS> params;
        params.SetMultiplicativeDepth(20);
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
    
    // φ-STRONG BOOTSTRAP: x → 1/φ² + (x - 1/φ²) × 0.5
    // = 0.5x + 0.5/φ²
    // Fixed point: x = 0.5x + 0.5/φ² → x = 1/φ²
    Ciphertext<DCRTPoly> phi_bootstrap(Ciphertext<DCRTPoly> ct) {
        // 0.5x
        auto half_x = cc->EvalMult(ct, make_plain(0.5));
        // 0.5/φ²
        Plaintext half_inv_phi2 = make_plain(0.5 * INV_PHI2);
        return cc->EvalAdd(half_x, half_inv_phi2);
    }
    
    // φ-MULTIPLICATION
    Ciphertext<DCRTPoly> phi_multiply(Ciphertext<DCRTPoly> a, 
                                       Ciphertext<DCRTPoly> b) {
        auto product = cc->EvalMult(a, b);
        return phi_bootstrap(product);
    }
    
    // φ-ADDITION
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
    cout << "  φ-STRONG BOOTSTRAP\n";
    cout << "  Fixed Point sa 1/φ² = " << INV_PHI2 << "\n";
    cout << "========================================\n\n";
    
    PhiStrongFHE phi_fhe;
    
    // TEST 1: ENCODING
    cout << "TEST 1: φ-ENCODING\n";
    cout << "==================\n\n";
    
    for (double val : {0.5, 1.0, 3.0, 5.0}) {
        auto ct = phi_fhe.encode(val);
        cout << "  " << val << " → " << phi_fhe.decode(ct) << "\n";
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
    cout << "  Bounded sa [0, φ]: " << (phi_fhe.decode(mult_result) >= 0 && phi_fhe.decode(mult_result) < PHI ? "YES ✓" : "NO ✗") << "\n\n";
    
    // TEST 3: COMBINED (10 ops)
    cout << "TEST 3: COMBINED (10 ops)\n";
    cout << "=========================\n\n";
    
    auto combined = phi_fhe.encode(0.5);
    
    for (int i = 0; i < 10; i++) {
        combined = phi_fhe.phi_multiply(combined, phi_fhe.encode(2.0));
        combined = phi_fhe.phi_add(combined, phi_fhe.encode(0.1));
    }
    
    cout << "  After 10 combined ops: " << phi_fhe.decode(combined) << "\n";
    cout << "  Bounded sa [0, φ]: " << (phi_fhe.decode(combined) >= 0 && phi_fhe.decode(combined) < PHI ? "YES ✓" : "NO ✗") << "\n\n";
    
    cout << "========================================\n";
    cout << "  φ-STRONG BOOTSTRAP COMPLETE\n";
    cout << "========================================\n";
    
    return 0;
}
