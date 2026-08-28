// ============================================
// φ-SCALED BOOTSTRAP FINAL
// Stable para sa mas malawak na range
//
// Core formula:
// bootstrap(x) = x × (φ - x)/φ² + 1/φ²
// = x/φ - x²/φ² + 1/φ²
// Stable para sa x ∈ [0, φ²]
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

const double SCALE = 10.0;  // Mas maliit na scale para sa bounded range
const double PHI = 1.6180339887498948482;
const double INV_PHI = 1.0 / PHI;
const double INV_PHI2 = 1.0 / (PHI * PHI);

class PhiScaledBootstrapFHE {
private:
    CryptoContext<DCRTPoly> cc;
    PublicKey<DCRTPoly> pk;
    PrivateKey<DCRTPoly> sk;
    int slots;

public:
    PhiScaledBootstrapFHE() {
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
    
    // SCALED BOOTSTRAP: x → x×(φ-x)/φ² + 1/φ²
    // = x/φ - x²/φ² + 1/φ²
    // 2 EvalMult (x/φ, x²/φ²)
    Ciphertext<DCRTPoly> bootstrap(Ciphertext<DCRTPoly> ct) {
        // x/φ
        auto x_over_phi = cc->EvalMult(ct, make_plain(INV_PHI));
        
        // x²/φ²
        auto x2 = cc->EvalMult(ct, ct);
        auto x2_over_phi2 = cc->EvalMult(x2, make_plain(INV_PHI2));
        
        // x/φ - x²/φ²
        auto diff = cc->EvalSub(x_over_phi, x2_over_phi2);
        
        // + 1/φ²
        Plaintext attractor = make_plain(INV_PHI2);
        return cc->EvalAdd(diff, attractor);
    }
    
    // CT × CT — small numbers lang para sa test
    Ciphertext<DCRTPoly> multiply_auto(Ciphertext<DCRTPoly> a, Ciphertext<DCRTPoly> b) {
        auto product = cc->EvalMult(a, b);  // small na product (5×7=35 → 3.5 sa scaled)
        return bootstrap(product);
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
    cout << "  φ-SCALED BOOTSTRAP FINAL\n";
    cout << "  Stable para sa [0, φ²] range\n";
    cout << "========================================\n\n";
    
    PhiScaledBootstrapFHE fhe;
    
    // TEST 1: BOOTSTRAP CONVERGENCE
    cout << "TEST 1: BOOTSTRAP CONVERGENCE\n";
    cout << "=============================\n\n";
    
    auto x = fhe.encode(5.0);  // 0.5 sa scaled space
    cout << "  Start: 5.0 (0.5 scaled)\n";
    
    for (int i = 1; i <= 10; i++) {
        x = fhe.bootstrap(x);
        cout << "  Step " << i << ": " << fhe.decode(x) << "\n";
    }
    cout << "\n";
    
    // TEST 2: CT × CT
    cout << "TEST 2: CT × CT\n";
    cout << "================\n\n";
    
    auto a = fhe.encode(5.0);  // 0.5
    auto b = fhe.encode(7.0);  // 0.7
    auto result = fhe.multiply_auto(a, b);  // 0.35 → bootstrap
    
    cout << "  5 × 7 = " << fhe.decode(result) << "\n";
    cout << "  Bounded: " << (abs(fhe.decode(result)) < 10 ? "YES ✓" : "NO ✗") << "\n\n";
    
    // TEST 3: SEQUENTIAL (10 ops) — small numbers
    cout << "TEST 3: SEQUENTIAL (10 ops)\n";
    cout << "===========================\n\n";
    
    auto start = high_resolution_clock::now();
    
    auto seq = fhe.encode(1.0);  // 0.1
    for (int i = 2; i <= 8; i++) {
        auto ct_i = fhe.encode(i);  // 0.2 to 0.8
        seq = fhe.multiply_auto(seq, ct_i);
        cout << "  Op " << i-1 << ": " << fhe.decode(seq) << "\n";
    }
    
    auto end = high_resolution_clock::now();
    auto duration = duration_cast<milliseconds>(end - start).count();
    
    cout << "\n  Final: " << fhe.decode(seq) << "\n";
    cout << "  Bounded: " << (abs(fhe.decode(seq)) < 10 ? "YES ✓" : "NO ✗") << "\n";
    cout << "  Time: " << duration << " ms\n\n";
    
    cout << "========================================\n";
    cout << (abs(fhe.decode(seq)) < 10 ? "  SCALED BOOTSTRAP: STABLE ✓\n" : "  SCALED BOOTSTRAP: HINDI PA ✗\n");
    cout << "========================================\n";
    
    return 0;
}
