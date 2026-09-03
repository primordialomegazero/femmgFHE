// ============================================
// φ-DEPTH5 CHAINED — 100/100 TARGET
//
// Depth 5 para sa precision headroom
// Integer encoding: φ×1000 = 1618
// Chained 100 additions nang walang drift
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
    cout << "  φ-DEPTH5 CHAINED\n";
    cout << "========================================\n\n";

    CCParams<CryptoContextCKKSRNS> parameters;
    parameters.SetMultiplicativeDepth(5);
    parameters.SetScalingModSize(50);
    parameters.SetBatchSize(16);
    parameters.SetSecurityLevel(HEStd_128_classic);

    CryptoContext<DCRTPoly> cc = GenCryptoContext(parameters);
    cc->Enable(PKE);
    cc->Enable(KEYSWITCH);
    cc->Enable(LEVELEDSHE);

    auto keyPair = cc->KeyGen();

    const int PHI_INT = 1618;      // φ × 1000
    const int PHI_INV_INT = 618;   // φ⁻¹ × 1000
    const int SCALE = 1000;

    cout << "  ✅ CKKS initialized (depth 5!)\n";
    cout << "  Integer encoding: φ×1000 = 1618\n\n";

    auto encrypt_int = [&](int bit) {
        int val = (bit == 0) ? PHI_INT : PHI_INV_INT;
        vector<double> v(16, (double)val);
        Plaintext pt = cc->MakeCKKSPackedPlaintext(v);
        return cc->Encrypt(keyPair.publicKey, pt);
    };

    auto decrypt_avg = [&](const Ciphertext<DCRTPoly>& ct) {
        Plaintext result_pt;
        cc->Decrypt(keyPair.secretKey, ct, &result_pt);
        result_pt->SetLength(16);
        double sum = 0.0;
        for (int i = 0; i < 16; i++) sum += result_pt->GetCKKSPackedValue()[i].real();
        return sum / 16.0;
    };

    // EXACT DECODE: integer-based
    auto exact_decode = [&](double val) {
        int int_val = (int)round(val / SCALE);
        int mod_phi = int_val % PHI_INT;
        if (mod_phi < 0) mod_phi += PHI_INT;
        
        // Zero detection
        if (mod_phi < 50 || mod_phi > PHI_INT - 50) return 0;
        
        // Half-split
        return (mod_phi < PHI_INT / 2) ? 1 : 0;
    };

    // ============================================
    // TEST 1: SINGLE ADDITIONS (8/8 CHECK)
    // ============================================

    cout << "========================================\n";
    cout << "  SINGLE ADDITIONS (8/8 CHECK)\n";
    cout << "========================================\n\n";

    cout << "  A B Cin | Sum | Expected | Match?\n";
    cout << "  --------|-----|----------|--------\n";

    int single_match = 0;
    for (int A : {0, 1}) {
        for (int B : {0, 1}) {
            for (int Cin : {0, 1}) {
                auto ct_a = encrypt_int(A);
                auto ct_b = encrypt_int(B);
                auto ct_cin = encrypt_int(Cin);
                
                auto ct_sum = cc->EvalAdd(cc->EvalAdd(ct_a, ct_b), ct_cin);
                double avg = decrypt_avg(ct_sum);
                int decoded = exact_decode(avg);
                
                int expected = (A + B + Cin) % 2;
                bool match = (decoded == expected);
                single_match += match;
                
                cout << "  " << A << " " << B << " " << Cin << " | "
                     << setw(3) << decoded << " | "
                     << setw(6) << expected << " | "
                     << (match ? "✅" : "❌") << "\n";
            }
        }
    }
    cout << "\n  Single: " << single_match << "/8\n\n";

    // ============================================
    // TEST 2: CHAINED (100 STEPS)
    // ============================================

    cout << "========================================\n";
    cout << "  CHAINED (DEPTH 5)\n";
    cout << "========================================\n\n";

    auto ct_acc = encrypt_int(0);
    auto ct_add = encrypt_int(1);

    auto start = high_resolution_clock::now();
    int chain_match = 0;

    for (int i = 0; i < 100; i++) {
        ct_acc = cc->EvalAdd(ct_acc, ct_add);
        double avg = decrypt_avg(ct_acc);
        int decoded = exact_decode(avg);
        int expected = (i + 1) % 2;
        chain_match += (decoded == expected);
    }

    auto end = high_resolution_clock::now();
    auto time = duration_cast<milliseconds>(end - start).count();

    cout << "  Match: " << chain_match << "/100\n";
    cout << "  Time: " << time << " ms\n";
    cout << "  Level: " << ct_acc->GetLevel() << "\n\n";

    // ============================================
    // SUMMARY
    // ============================================

    cout << "========================================\n";
    cout << "  DEPTH5 CHAINED COMPLETE\n";
    cout << "========================================\n\n";
    cout << "  ✅ Single: " << single_match << "/8\n";
    cout << "  ✅ Chained: " << chain_match << "/100\n";
    cout << "  ✅ Level: " << ct_acc->GetLevel() << "\n\n";

    return 0;
}
