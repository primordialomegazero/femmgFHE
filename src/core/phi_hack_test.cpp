// ============================================
// φ-HACK TEST — Maliit lang
// Test: EvalMult sa plaintext vs ciphertext
// Test: Log space para iwas multiply
// Test: Fibonacci para iwas multiply
// ============================================

#include <iostream>
#include <vector>
#include <cmath>
#include <iomanip>
#include <chrono>
#include "pke/openfhe.h"

using namespace lbcrypto;
using namespace std;
using namespace std::chrono;

int main() {
    CCParams<CryptoContextCKKSRNS> parameters;
    parameters.SetMultiplicativeDepth(3);
    parameters.SetScalingModSize(59);
    parameters.SetBatchSize(4);
    parameters.SetSecurityLevel(HEStd_128_classic);

    CryptoContext<DCRTPoly> cc = GenCryptoContext(parameters);
    cc->Enable(PKE);
    cc->Enable(KEYSWITCH);
    cc->Enable(LEVELEDSHE);
    auto keyPair = cc->KeyGen();
    cc->EvalMultKeyGen(keyPair.secretKey);

    const double PHI = 1.6180339887498948482;
    const double LN_PHI = log(PHI);

    cout << "========================================\n";
    cout << "  φ-HACK TEST\n";
    cout << "========================================\n\n";

    // ============================================
    // TEST 1: Plaintext multiplication
    // Ilang beses kaya bago maubos ang depth?
    // ============================================
    cout << "--- TEST 1: Plaintext Multiplication ---\n";
    
    vector<double> v(4, 0.0);
    v[0] = 1.5;
    v[1] = 2.5;
    v[2] = 3.5;
    v[3] = 4.5;
    Plaintext pt = cc->MakeCKKSPackedPlaintext(v);
    auto ct = cc->Encrypt(keyPair.publicKey, pt);
    
    vector<double> mult_v(4, 0.0);
    mult_v[0] = 2.0;
    mult_v[1] = 2.0;
    mult_v[2] = 2.0;
    mult_v[3] = 2.0;
    Plaintext pt_mult = cc->MakeCKKSPackedPlaintext(mult_v);
    
    cout << "  Initial level: " << ct->GetLevel() << "\n";
    
    int mult_count = 0;
    bool ok = true;
    while (ok && mult_count < 10) {
        try {
            ct = cc->EvalMult(ct, pt_mult);
            mult_count++;
            if (mult_count <= 5) {
                cout << "  After " << mult_count << " mults: level=" << ct->GetLevel() << "\n";
            }
        } catch (...) {
            ok = false;
        }
    }
    
    cout << "  Total plaintext mults bago bumagsak: " << mult_count << "\n";
    cout << "  Final level: " << ct->GetLevel() << "\n\n";

    // ============================================
    // TEST 2: Ciphertext multiplication
    // Para sa comparison
    // ============================================
    cout << "--- TEST 2: Ciphertext Multiplication ---\n";
    
    vector<double> v2(4, 0.0);
    v2[0] = 1.5;
    v2[1] = 2.5;
    v2[2] = 3.5;
    v2[3] = 4.5;
    Plaintext pt2 = cc->MakeCKKSPackedPlaintext(v2);
    auto ct2 = cc->Encrypt(keyPair.publicKey, pt2);
    
    vector<double> mult2_v(4, 0.0);
    mult2_v[0] = 2.0;
    mult2_v[1] = 2.0;
    mult2_v[2] = 2.0;
    mult2_v[3] = 2.0;
    Plaintext pt2_mult = cc->MakeCKKSPackedPlaintext(mult2_v);
    auto ct2_mult = cc->Encrypt(keyPair.publicKey, pt2_mult);
    
    cout << "  Initial level: " << ct2->GetLevel() << "\n";
    
    int ct_mult_count = 0;
    ok = true;
    while (ok && ct_mult_count < 5) {
        try {
            ct2 = cc->EvalMult(ct2, ct2_mult);
            ct_mult_count++;
            cout << "  After " << ct_mult_count << " mults: level=" << ct2->GetLevel() << "\n";
        } catch (...) {
            ok = false;
        }
    }
    
    cout << "  Total ciphertext mults bago bumagsak: " << ct_mult_count << "\n\n";

    // ============================================
    // TEST 3: Log space — walang multiply
    // ============================================
    cout << "--- TEST 3: Log Space (Walang Multiply) ---\n";
    
    vector<double> v3(4, 0.0);
    v3[0] = 1.0;  // log value
    v3[1] = 2.0;
    v3[2] = 3.0;
    v3[3] = 4.0;
    Plaintext pt3 = cc->MakeCKKSPackedPlaintext(v3);
    auto ct3 = cc->Encrypt(keyPair.publicKey, pt3);
    
    vector<double> add_v(4, 0.0);
    add_v[0] = 0.5;  // multiply ng φ^0.5 sa normal
    add_v[1] = -0.3; // divide ng φ^0.3 sa normal
    add_v[2] = 0.8;  // multiply ng φ^0.8 sa normal
    add_v[3] = -0.2; // divide ng φ^0.2 sa normal
    Plaintext pt3_add = cc->MakeCKKSPackedPlaintext(add_v);
    
    cout << "  Initial level: " << ct3->GetLevel() << "\n";
    
    auto start = high_resolution_clock::now();
    
    for (int i = 0; i < 1000; i++) {
        ct3 = cc->EvalAdd(ct3, pt3_add);
    }
    
    auto end = high_resolution_clock::now();
    auto time = duration_cast<milliseconds>(end - start).count();
    
    cout << "  1000 EvalAdd sa log space\n";
    cout << "  Time: " << time << " ms\n";
    cout << "  Final level: " << ct3->GetLevel() << "\n";
    
    Plaintext pt3_out;
    cc->Decrypt(keyPair.secretKey, ct3, &pt3_out);
    pt3_out->SetLength(4);
    auto res3 = pt3_out->GetCKKSPackedValue();
    
    cout << "  Final log values: ";
    for (int i = 0; i < 4; i++) {
        cout << res3[i].real() << " ";
    }
    cout << "\n\n";

    return 0;
}
