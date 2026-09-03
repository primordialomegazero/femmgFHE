// ============================================
// φ-FRACTAL ALU FIX — 16 PARALLEL ALUs
//
// Tamang alternating inputs para sa XOR3
//
// Author: Dan Fernandez / Primordial Omega Zero
// ============================================

#include <iostream>
#include <vector>
#include <cmath>
#include <complex>
#include <iomanip>
#include <chrono>

#include "pke/openfhe.h"

using namespace lbcrypto;
using namespace std;
using namespace std::chrono;

int main() {
    cout << "========================================\n";
    cout << "  φ-FRACTAL ALU FIX\n";
    cout << "========================================\n\n";

    CCParams<CryptoContextCKKSRNS> parameters;
    parameters.SetMultiplicativeDepth(1);
    parameters.SetScalingModSize(59);
    parameters.SetBatchSize(16);
    parameters.SetSecurityLevel(HEStd_128_classic);

    CryptoContext<DCRTPoly> cc = GenCryptoContext(parameters);
    cc->Enable(PKE);
    cc->Enable(KEYSWITCH);
    cc->Enable(LEVELEDSHE);

    auto keyPair = cc->KeyGen();

    const double PHI = 1.6180339887498948482;
    const double HALF_PHI = PHI / 2.0;
    const double TWO_PHI = 2.0 * PHI;

    cout << "  ✅ CKKS initialized (depth 1, modsize 59!)\n";
    cout << "  Fractal ALU: 16 parallel XOR3\n\n";

    auto xor3_decode = [&](double val) {
        double mod_2phi = fmod(val, TWO_PHI);
        if (mod_2phi < 0.1 || mod_2phi > TWO_PHI - 0.1) return 1;
        if (abs(mod_2phi - PHI) < 0.1) return 1;
        if (abs(mod_2phi - HALF_PHI) < 0.1 || abs(mod_2phi - PHI - HALF_PHI) < 0.1) return 0;
        double mod_phi = fmod(val, PHI);
        if (mod_phi < 0.1 || mod_phi > PHI - 0.1) return 1;
        return 0;
    };

    // ============================================
    // FRACTAL ALU — 16 PARALLEL
    // ============================================

    cout << "========================================\n";
    cout << "  16 PARALLEL ALUs (XOR3)\n";
    cout << "========================================\n\n";

    // Bawat ALU: A + B + Cin
    // A = alternating 0/1, B = 1, Cin = 0
    // Expected: A XOR B XOR Cin = (A + 1) % 2
    
    vector<double> A_vals(16, 0.0);
    vector<double> B_vals(16, 0.0);
    vector<double> Cin_vals(16, 0.0);
    
    for (int i = 0; i < 16; i++) {
        // A: alternating 0 at 1
        A_vals[i] = (i % 2 == 0) ? HALF_PHI : PHI;
        // B: lahat 1
        B_vals[i] = PHI;
        // Cin: lahat 0
        Cin_vals[i] = HALF_PHI;
    }
    
    Plaintext pt_a = cc->MakeCKKSPackedPlaintext(A_vals);
    Plaintext pt_b = cc->MakeCKKSPackedPlaintext(B_vals);
    Plaintext pt_cin = cc->MakeCKKSPackedPlaintext(Cin_vals);
    
    auto ct_a = cc->Encrypt(keyPair.publicKey, pt_a);
    auto ct_b = cc->Encrypt(keyPair.publicKey, pt_b);
    auto ct_cin = cc->Encrypt(keyPair.publicKey, pt_cin);
    
    auto ct_sum = cc->EvalAdd(cc->EvalAdd(ct_a, ct_b), ct_cin);
    
    Plaintext result_pt;
    cc->Decrypt(keyPair.secretKey, ct_sum, &result_pt);
    result_pt->SetLength(16);
    
    int match = 0;
    cout << "  Slot | A | B | Cin | Sum | Expected | Match?\n";
    cout << "  -----|---|---|-----|-----|----------|--------\n";
    
    for (int i = 0; i < 16; i++) {
        double val = result_pt->GetCKKSPackedValue()[i].real();
        int decoded = xor3_decode(val);
        int expected = (i % 2 + 1 + 0) % 2;
        bool ok = (decoded == expected);
        match += ok;
        
        cout << "  " << setw(4) << i << " | "
             << (i % 2 == 0 ? "0" : "1") << " | "
             << "1 | "
             << "0 | "
             << setw(3) << decoded << " | "
             << setw(8) << expected << " | "
             << (ok ? "✅" : "❌") << "\n";
    }
    
    cout << "\n  Fractal ALU Match: " << match << "/16\n";
    cout << "  Level: " << ct_sum->GetLevel() << "\n\n";

    // ============================================
    // BATCH XOR3 — 16 DIFFERENT COMBINATIONS
    // ============================================

    cout << "========================================\n";
    cout << "  BATCH XOR3 — 16 COMBINATIONS\n";
    cout << "========================================\n\n";

    vector<double> A_batch(16, 0.0);
    vector<double> B_batch(16, 0.0);
    vector<double> Cin_batch(16, 0.0);
    
    for (int i = 0; i < 16; i++) {
        A_batch[i] = ((i >> 2) & 1) ? PHI : HALF_PHI;
        B_batch[i] = ((i >> 1) & 1) ? PHI : HALF_PHI;
        Cin_batch[i] = (i & 1) ? PHI : HALF_PHI;
    }
    
    Plaintext pt_ab = cc->MakeCKKSPackedPlaintext(A_batch);
    Plaintext pt_bb = cc->MakeCKKSPackedPlaintext(B_batch);
    Plaintext pt_cb = cc->MakeCKKSPackedPlaintext(Cin_batch);
    
    auto ct_ab = cc->Encrypt(keyPair.publicKey, pt_ab);
    auto ct_bb = cc->Encrypt(keyPair.publicKey, pt_bb);
    auto ct_cb = cc->Encrypt(keyPair.publicKey, pt_cb);
    
    auto ct_batch_sum = cc->EvalAdd(cc->EvalAdd(ct_ab, ct_bb), ct_cb);
    
    Plaintext batch_pt;
    cc->Decrypt(keyPair.secretKey, ct_batch_sum, &batch_pt);
    batch_pt->SetLength(16);
    
    int batch_match = 0;
    cout << "  Slot | A | B | Cin | Sum | Expected | Match?\n";
    cout << "  -----|---|---|-----|-----|----------|--------\n";
    
    for (int i = 0; i < 16; i++) {
        double val = batch_pt->GetCKKSPackedValue()[i].real();
        int decoded = xor3_decode(val);
        int A = (i >> 2) & 1;
        int B = (i >> 1) & 1;
        int Cin = i & 1;
        int expected = (A + B + Cin) % 2;
        bool ok = (decoded == expected);
        batch_match += ok;
        
        cout << "  " << setw(4) << i << " | "
             << A << " | "
             << B << " | "
             << Cin << " | "
             << setw(3) << decoded << " | "
             << setw(8) << expected << " | "
             << (ok ? "✅" : "❌") << "\n";
    }
    
    cout << "\n  Batch XOR3 Match: " << batch_match << "/16\n";
    cout << "  Level: " << ct_batch_sum->GetLevel() << "\n\n";

    return 0;
}
