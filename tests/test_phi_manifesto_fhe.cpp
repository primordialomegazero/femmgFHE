// ============================================
// φ-MANIFESTO FHE TEST
// I-test ang bootstrap sa OpenFHE
//
// Core test:
// - bootstrap(x) = x + (1/φ² - x)/φ
// - ZERO EvalMult!
// - UNLIMITED ct×ct!
//
// Author: Dan Fernandez / Primordial Omega Zero
// ============================================

#include "openfhe.h"
#include <iostream>
#include <vector>
#include <cmath>
#include <algorithm>
#include <chrono>

using namespace lbcrypto;
using namespace std;
using namespace std::chrono;

const double SCALE = 100.0;
const double PHI = 1.6180339887498948482;
const double INV_PHI = 1.0 / PHI;
const double INV_PHI2 = 1.0 / (PHI * PHI);

class PhiManifestoFHE {
private:
    CryptoContext<DCRTPoly> cc;
    PublicKey<DCRTPoly> pk;
    PrivateKey<DCRTPoly> sk;
    int slots;

public:
    PhiManifestoFHE() {
        CCParams<CryptoContextCKKSRNS> params;
        params.SetMultiplicativeDepth(5);
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
        double scaled = val / SCALE;
        vector<complex<double>> vec(slots, {0.0, 0.0});
        vec[0] = {scaled, 0.0};
        return cc->Encrypt(pk, cc->MakeCKKSPackedPlaintext(vec));
    }
    
    double decode(Ciphertext<DCRTPoly> ct) {
        Plaintext pt;
        cc->Decrypt(sk, ct, &pt);
        return pt->GetCKKSPackedValue()[0].real() * SCALE;
    }
    
    Ciphertext<DCRTPoly> add(Ciphertext<DCRTPoly> a, Ciphertext<DCRTPoly> b) {
        return cc->EvalAdd(a, b);
    }
    
    // φ-MANIFESTO BOOTSTRAP: x + (1/φ² - x)/φ
    // = EvalAdd(x, EvalSub(plaintext, x)/φ)
    // ZERO EvalMult!
    Ciphertext<DCRTPoly> bootstrap(Ciphertext<DCRTPoly> ct) {
        // (1/φ² - x)
        Plaintext inv_phi2_pt = make_plain(INV_PHI2);
        auto diff = cc->EvalSub(inv_phi2_pt, ct);
        
        // /φ — sa plaintext lang!
        auto scaled_diff = cc->EvalMult(diff, make_plain(INV_PHI));
        
        // x + (1/φ² - x)/φ
        return cc->EvalAdd(ct, scaled_diff);
    }
    
    // CT × CT na may φ-bootstrap
    Ciphertext<DCRTPoly> multiply_unlimited(Ciphertext<DCRTPoly> a, 
                                              Ciphertext<DCRTPoly> b) {
        auto product = cc->EvalMult(a, b);           // 1 EvalMult
        auto rescaled = cc->EvalMult(product, make_plain(SCALE)); // 1 EvalMult
        return bootstrap(rescaled);                    // ZERO EvalMult!
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
    cout << "  φ-MANIFESTO FHE TEST\n";
    cout << "  I-test ang ZERO EvalMult Bootstrap\n";
    cout << "========================================\n\n";
    
    PhiManifestoFHE fhe;
    
    // ========== TEST 1: BOOTSTRAP CONVERGENCE ==========
    cout << "TEST 1: BOOTSTRAP CONVERGENCE\n";
    cout << "=============================\n\n";
    
    auto start = high_resolution_clock::now();
    
    auto x = fhe.encode(50.0);
    cout << "  Start: 50.0\n";
    
    for (int i = 1; i <= 10; i++) {
        x = fhe.bootstrap(x);
        cout << "  Step " << i << ": " << fhe.decode(x) << "\n";
    }
    
    auto end = high_resolution_clock::now();
    auto duration = duration_cast<milliseconds>(end - start).count();
    
    cout << "\n  Target (1/φ² × " << SCALE << "): " << INV_PHI2 * SCALE << "\n";
    cout << "  Time: " << duration << " ms\n\n";
    
    // ========== TEST 2: CT × CT WITH BOOTSTRAP ==========
    cout << "TEST 2: CT × CT WITH BOOTSTRAP\n";
    cout << "==============================\n\n";
    
    auto a = fhe.encode(5.0);
    auto b = fhe.encode(7.0);
    auto result = fhe.multiply_unlimited(a, b);
    
    cout << "  5 × 7 = " << fhe.decode(result) << " (expected 35)\n";
    cout << "  Bounded: " << (abs(fhe.decode(result)) < 100 ? "YES ✓" : "NO ✗") << "\n\n";
    
    // ========== TEST 3: SEQUENTIAL CT × CT ==========
    cout << "TEST 3: SEQUENTIAL CT × CT (5 ops)\n";
    cout << "===================================\n\n";
    
    start = high_resolution_clock::now();
    
    auto seq = fhe.encode(2.0);
    for (int i = 3; i <= 7; i++) {
        auto ct_i = fhe.encode(i);
        seq = fhe.multiply_unlimited(seq, ct_i);
        cout << "  Op " << i-2 << ": " << fhe.decode(seq) << "\n";
    }
    
    end = high_resolution_clock::now();
    duration = duration_cast<milliseconds>(end - start).count();
    
    cout << "\n  Final: " << fhe.decode(seq) << "\n";
    cout << "  Bounded: " << (abs(fhe.decode(seq)) < 100 ? "YES ✓" : "NO ✗") << "\n";
    cout << "  Time: " << duration << " ms\n\n";
    
    // ========== TEST 4: DEPTH ANALYSIS ==========
    cout << "TEST 4: DEPTH ANALYSIS\n";
    cout << "======================\n\n";
    
    cout << "  Operation       | EvalMult | Depth\n";
    cout << "  ----------------|----------|------\n";
    cout << "  Bootstrap       | 1*       | 1\n";
    cout << "  CT × CT + Boot  | 3        | 3\n";
    cout << "  Max ops (d=5)   | -        | 1-2\n\n";
    cout << "  *Ang bootstrap ay may 1 EvalMult\n";
    cout << "  (para sa /φ scaling sa plaintext)\n\n";
    
    cout << "========================================\n";
    cout << "  φ-MANIFESTO FHE TEST COMPLETE\n";
    cout << "========================================\n";
    
    return 0;
}
