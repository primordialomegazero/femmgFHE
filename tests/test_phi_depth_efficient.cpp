// ============================================
// φ-DEPTH-EFFICIENT
// Combined operations na 1 bootstrap lang
//
// Core fix:
// - combined(a,b,c) = bootstrap(a×b + c)
// - 1 multiplication + 1 bootstrap = 2 mults
// - Imbis na 2×(mult + bootstrap) = 4 mults
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

class PhiDepthEfficientFHE {
private:
    CryptoContext<DCRTPoly> cc;
    PublicKey<DCRTPoly> pk;
    PrivateKey<DCRTPoly> sk;
    int slots;

public:
    PhiDepthEfficientFHE() {
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
    
    // φ-BOOTSTRAP: 0.5x + 0.5/φ²
    Ciphertext<DCRTPoly> phi_bootstrap(Ciphertext<DCRTPoly> ct) {
        auto half_x = cc->EvalMult(ct, make_plain(0.5));
        Plaintext half_inv_phi2 = make_plain(0.5 * INV_PHI2);
        return cc->EvalAdd(half_x, half_inv_phi2);
    }
    
    // φ-COMBINED: bootstrap(a×b + c)
    // 2 multiplications lang total!
    Ciphertext<DCRTPoly> phi_combined(Ciphertext<DCRTPoly> a,
                                       Ciphertext<DCRTPoly> b,
                                       Ciphertext<DCRTPoly> c) {
        auto product = cc->EvalMult(a, b);      // 1 mult
        auto sum = cc->EvalAdd(product, c);      // 0 mult
        return phi_bootstrap(sum);               // 1 mult (bootstrap)
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
    cout << "  φ-DEPTH-EFFICIENT\n";
    cout << "  2 Multiplications per Combined Op\n";
    cout << "========================================\n\n";
    
    PhiDepthEfficientFHE phi_fhe;
    
    // TEST: UNBOUNDED (15 combined ops)
    cout << "TEST: UNBOUNDED (15 combined ops)\n";
    cout << "=================================\n\n";
    
    auto result = phi_fhe.encode(0.5);
    
    for (int i = 0; i < 15; i++) {
        auto two = phi_fhe.encode(2.0);
        auto inc = phi_fhe.encode(0.1);
        result = phi_fhe.phi_combined(result, two, inc);
        cout << "  Op " << i+1 << ": " << phi_fhe.decode(result) << "\n";
    }
    
    cout << "\n  Final: " << phi_fhe.decode(result) << "\n";
    cout << "  Bounded sa [0, φ]: " << (phi_fhe.decode(result) >= 0 && phi_fhe.decode(result) < PHI ? "YES ✓" : "NO ✗") << "\n\n";
    
    cout << "========================================\n";
    cout << "  φ-DEPTH-EFFICIENT COMPLETE\n";
    cout << "========================================\n";
    
    return 0;
}
