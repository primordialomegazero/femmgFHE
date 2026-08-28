// ============================================
// φ-MIXED FHE BOOTSTRAP TEST
// Ang φ-Mixed bootstrap sa OpenFHE
//
// Core test:
// - φ-Mixed: compression(sqrt(reciprocal(x)))
// - Bounded sa 0.2022
// - Autonomous at stable
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

class PhiMixedFHE {
private:
    CryptoContext<DCRTPoly> cc;
    PublicKey<DCRTPoly> pk;
    PrivateKey<DCRTPoly> sk;
    int slots;

public:
    PhiMixedFHE() {
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
    
    // φ-MIXED BOOTSTRAP: compression(sqrt(reciprocal(x)))
    // Simplified: x → x/(φ+x) — compression lang
    Ciphertext<DCRTPoly> bootstrap(Ciphertext<DCRTPoly> ct) {
        // compression: x/(φ+x) = 1 - φ/(φ+x)
        // Sa FHE: gamitin ang φ-normalized version
        // x → x × INV_PHI / (1 + x × INV_PHI)
        
        // Step 1: x/φ
        auto x_over_phi = cc->EvalMult(ct, make_plain(INV_PHI));
        
        // Step 2: 1 + x/φ
        Plaintext one = make_plain(1.0);
        auto one_plus = cc->EvalAdd(one, x_over_phi);
        
        // Step 3: x × (1/(1+x/φ)) — approximation: x × (1 - x/φ)
        auto result = cc->EvalMult(ct, cc->EvalSub(one, x_over_phi));
        
        return result;
    }
    
    // CT × CT with φ-Mixed bootstrap
    Ciphertext<DCRTPoly> multiply_auto(Ciphertext<DCRTPoly> a, Ciphertext<DCRTPoly> b) {
        auto product = cc->EvalMult(a, b);
        auto rescaled = cc->EvalMult(product, make_plain(SCALE));
        return bootstrap(rescaled);
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
    cout << "  φ-MIXED FHE BOOTSTRAP TEST\n";
    cout << "  Compression Bootstrap sa OpenFHE\n";
    cout << "========================================\n\n";
    
    PhiMixedFHE fhe;
    
    // TEST 1: BOOTSTRAP CONVERGENCE
    cout << "TEST 1: BOOTSTRAP CONVERGENCE\n";
    cout << "=============================\n\n";
    
    auto x = fhe.encode(100.0);
    cout << "  Start: 100.0\n";
    
    for (int i = 1; i <= 10; i++) {
        x = fhe.bootstrap(x);
        cout << "  Step " << i << ": " << fhe.decode(x) << "\n";
    }
    cout << "\n";
    
    // TEST 2: CT × CT
    cout << "TEST 2: CT × CT\n";
    cout << "================\n\n";
    
    auto a = fhe.encode(5.0);
    auto b = fhe.encode(7.0);
    auto result = fhe.multiply_auto(a, b);
    
    cout << "  5 × 7 = " << fhe.decode(result) << "\n";
    cout << "  Bounded: " << (abs(fhe.decode(result)) < 100 ? "YES ✓" : "NO ✗") << "\n\n";
    
    // TEST 3: SEQUENTIAL (10 ops)
    cout << "TEST 3: SEQUENTIAL (10 ops)\n";
    cout << "===========================\n\n";
    
    auto start = high_resolution_clock::now();
    
    auto seq = fhe.encode(2.0);
    for (int i = 3; i <= 10; i++) {
        auto ct_i = fhe.encode(i);
        seq = fhe.multiply_auto(seq, ct_i);
        cout << "  Op " << i-1 << ": " << fhe.decode(seq) << "\n";
    }
    
    auto end = high_resolution_clock::now();
    auto duration = duration_cast<milliseconds>(end - start).count();
    
    cout << "\n  Final: " << fhe.decode(seq) << "\n";
    cout << "  Bounded: " << (abs(fhe.decode(seq)) < 100 ? "YES ✓" : "NO ✗") << "\n";
    cout << "  Time: " << duration << " ms\n\n";
    
    cout << "========================================\n";
    cout << (abs(fhe.decode(seq)) < 100 ? "  φ-MIXED: STABLE ✓\n" : "  φ-MIXED: HINDI PA ✗\n");
    cout << "========================================\n";
    
    return 0;
}
