// ============================================
// φ-MANIFESTO DEPTH 30
// Mas malalim na depth para sa ct×ct
//
// Core fix:
// - Depth 30 para sa 10+ ct×ct operations
// - φ-Manifesto bootstrap (1 EvalMult)
// - Attractor reset para sa bounding
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

class PhiManifestoD30FHE {
private:
    CryptoContext<DCRTPoly> cc;
    PublicKey<DCRTPoly> pk;
    PrivateKey<DCRTPoly> sk;
    int slots;

public:
    PhiManifestoD30FHE() {
        CCParams<CryptoContextCKKSRNS> params;
        params.SetMultiplicativeDepth(30);  // MAS MALALIM!
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
    Ciphertext<DCRTPoly> bootstrap(Ciphertext<DCRTPoly> ct) {
        Plaintext inv_phi2_pt = make_plain(INV_PHI2);
        auto diff = cc->EvalSub(inv_phi2_pt, ct);
        auto scaled_diff = cc->EvalMult(diff, make_plain(INV_PHI));
        return cc->EvalAdd(ct, scaled_diff);
    }
    
    // CT × CT with bootstrap
    Ciphertext<DCRTPoly> multiply_ctct(Ciphertext<DCRTPoly> a, Ciphertext<DCRTPoly> b) {
        auto product = cc->EvalMult(a, b);                    // 1 EvalMult
        auto rescaled = cc->EvalMult(product, make_plain(SCALE)); // 1 EvalMult
        return bootstrap(rescaled);                             // 1 EvalMult
        // Total: 3 EvalMult per ct×ct
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
    cout << "  φ-MANIFESTO DEPTH 30\n";
    cout << "  Mas Malalim para sa ct×ct\n";
    cout << "========================================\n\n";
    
    PhiManifestoD30FHE fhe;
    
    // TEST 1: BOOTSTRAP CONVERGENCE
    cout << "TEST 1: BOOTSTRAP CONVERGENCE\n";
    cout << "=============================\n\n";
    
    auto x = fhe.encode(50.0);
    for (int i = 1; i <= 10; i++) {
        x = fhe.bootstrap(x);
        cout << "  Step " << i << ": " << fhe.decode(x) << "\n";
    }
    cout << "\n  Target: " << INV_PHI2 * SCALE << "\n\n";
    
    // TEST 2: CT × CT (10 ops)
    cout << "TEST 2: CT × CT (10 ops)\n";
    cout << "=========================\n\n";
    
    auto start = high_resolution_clock::now();
    
    auto result = fhe.encode(1.0);
    for (int i = 2; i <= 10; i++) {
        auto ct_i = fhe.encode(i);
        result = fhe.multiply_ctct(result, ct_i);
        cout << "  Op " << i-1 << ": " << fhe.decode(result) << "\n";
    }
    
    auto end = high_resolution_clock::now();
    auto duration = duration_cast<milliseconds>(end - start).count();
    
    cout << "\n  Final: " << fhe.decode(result) << "\n";
    cout << "  Bounded: " << (abs(fhe.decode(result)) < 100 ? "YES ✓" : "NO ✗") << "\n";
    cout << "  Time: " << duration << " ms\n\n";
    
    cout << "========================================\n";
    cout << "  φ-MANIFESTO DEPTH 30 COMPLETE\n";
    cout << "========================================\n";
    
    return 0;
}
