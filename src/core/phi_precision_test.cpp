// ============================================
// φ-PRECISION TEST — DEPTH 0 VS DEPTH 1
//
// I-compare ang precision ng CKKS sa depth 0 at 1
// para maunawaan ang rounding errors
//
// Author: Dan Fernandez / Primordial Omega Zero
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
    cout << "========================================\n";
    cout << "  φ-PRECISION TEST\n";
    cout << "========================================\n\n";

    const double PHI = 1.6180339887498948482;
    const double PHI_INV = 1.0 / PHI;
    const double W_L = PHI * PHI - PHI / 2.0;
    const double W_C = PHI;
    const double W_R = PHI_INV;

    // ============================================
    // DEPTH 0 TEST
    // ============================================

    cout << "========================================\n";
    cout << "  DEPTH 0\n";
    cout << "========================================\n\n";

    CCParams<CryptoContextCKKSRNS> params0;
    params0.SetMultiplicativeDepth(0);
    params0.SetScalingModSize(50);
    params0.SetBatchSize(16);
    params0.SetSecurityLevel(HEStd_128_classic);

    CryptoContext<DCRTPoly> cc0 = GenCryptoContext(params0);
    cc0->Enable(PKE);
    cc0->Enable(KEYSWITCH);
    cc0->Enable(LEVELEDSHE);

    auto kp0 = cc0->KeyGen();
    cc0->EvalMultKeyGen(kp0.secretKey);

    cout << "  Values na ie-encrypt:\n";
    cout << "  0.0, 0.618034, 1.61803, 1.80902\n\n";

    vector<double> test_vals = {0.0, PHI_INV, PHI, W_L, 2.236, 2.427, 3.427, 4.045};

    cout << "  Original | Encrypted | Decrypted | Error\n";
    cout << "  ---------|-----------|-----------|-------\n";

    for (double val : test_vals) {
        vector<double> v(16, val);
        Plaintext pt = cc0->MakeCKKSPackedPlaintext(v);
        auto ct = cc0->Encrypt(kp0.publicKey, pt);
        
        Plaintext result_pt;
        cc0->Decrypt(kp0.secretKey, ct, &result_pt);
        result_pt->SetLength(16);
        
        double sum = 0.0;
        for (int i = 0; i < 16; i++) sum += result_pt->GetCKKSPackedValue()[i].real();
        double avg = sum / 16.0;
        double error = abs(avg - val);
        
        cout << "  " << setw(9) << fixed << setprecision(6) << val << " | "
             << setw(9) << avg << " | "
             << setw(9) << error << "\n";
    }

    // ============================================
    // DEPTH 1 TEST
    // ============================================

    cout << "\n========================================\n";
    cout << "  DEPTH 1\n";
    cout << "========================================\n\n";

    CCParams<CryptoContextCKKSRNS> params1;
    params1.SetMultiplicativeDepth(1);
    params1.SetScalingModSize(50);
    params1.SetBatchSize(16);
    params1.SetSecurityLevel(HEStd_128_classic);

    CryptoContext<DCRTPoly> cc1 = GenCryptoContext(params1);
    cc1->Enable(PKE);
    cc1->Enable(KEYSWITCH);
    cc1->Enable(LEVELEDSHE);

    auto kp1 = cc1->KeyGen();
    cc1->EvalMultKeyGen(kp1.secretKey);

    cout << "  Original | Encrypted | Decrypted | Error\n";
    cout << "  ---------|-----------|-----------|-------\n";

    for (double val : test_vals) {
        vector<double> v(16, val);
        Plaintext pt = cc1->MakeCKKSPackedPlaintext(v);
        auto ct = cc1->Encrypt(kp1.publicKey, pt);
        
        Plaintext result_pt;
        cc1->Decrypt(kp1.secretKey, ct, &result_pt);
        result_pt->SetLength(16);
        
        double sum = 0.0;
        for (int i = 0; i < 16; i++) sum += result_pt->GetCKKSPackedValue()[i].real();
        double avg = sum / 16.0;
        double error = abs(avg - val);
        
        cout << "  " << setw(9) << fixed << setprecision(6) << val << " | "
             << setw(9) << avg << " | "
             << setw(9) << error << "\n";
    }

    // ============================================
    // EVALADD TEST — DEPTH 0
    // ============================================

    cout << "\n========================================\n";
    cout << "  EVALADD TEST — DEPTH 0\n";
    cout << "========================================\n\n";

    cout << "  Test: W_C + W_R = " << W_C << " + " << W_R << " = " << W_C + W_R << "\n";
    cout << "  Expected floor: " << floor(W_C + W_R) << "\n";
    cout << "  Expected mod 2: " << ((int)floor(W_C + W_R)) % 2 << "\n\n";

    vector<double> v1(16, W_C);
    vector<double> v2(16, W_R);
    
    Plaintext pt1 = cc0->MakeCKKSPackedPlaintext(v1);
    Plaintext pt2 = cc0->MakeCKKSPackedPlaintext(v2);
    
    auto ct1 = cc0->Encrypt(kp0.publicKey, pt1);
    auto ct2 = cc0->Encrypt(kp0.publicKey, pt2);
    
    auto sum = cc0->EvalAdd(ct1, ct2);
    
    Plaintext result_pt;
    cc0->Decrypt(kp0.secretKey, sum, &result_pt);
    result_pt->SetLength(16);
    
    double sum_val = 0.0;
    for (int i = 0; i < 16; i++) sum_val += result_pt->GetCKKSPackedValue()[i].real();
    double avg = sum_val / 16.0;
    
    cout << "  Decrypted sum: " << setprecision(10) << avg << "\n";
    cout << "  Expected: " << W_C + W_R << "\n";
    cout << "  Error: " << abs(avg - (W_C + W_R)) << "\n";
    cout << "  Floor: " << floor(avg) << "\n";
    cout << "  Mod 2: " << ((int)floor(avg)) % 2 << "\n\n";

    return 0;
}
