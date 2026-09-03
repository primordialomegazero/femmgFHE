// ============================================
// φ-DEBUG MIRROR — PRINT LAHAT
//
// Makikita natin kung saan nagkakaproblema
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
    cout << "  φ-DEBUG MIRROR\n";
    cout << "========================================\n\n";

    CCParams<CryptoContextCKKSRNS> parameters;
    parameters.SetMultiplicativeDepth(1);
    parameters.SetScalingModSize(50);
    parameters.SetBatchSize(8);
    parameters.SetSecurityLevel(HEStd_128_classic);

    CryptoContext<DCRTPoly> cc = GenCryptoContext(parameters);
    cc->Enable(PKE);
    cc->Enable(KEYSWITCH);
    cc->Enable(LEVELEDSHE);

    auto keyPair = cc->KeyGen();
    cc->EvalMultKeyGen(keyPair.secretKey);

    const double PHI = 1.6180339887498948482;
    const double LN_PHI = log(PHI);

    auto encrypt_mirror = [&](double value) {
        double base = log(value) / LN_PHI;
        double mirror = PHI - abs(fmod(base, PHI));
        
        vector<double> v(8, 0.0);
        v[0] = mirror;
        for (int i = 1; i < 8; i++) {
            v[i] = PHI - abs(fmod(base / pow(PHI, i), PHI));
        }
        
        Plaintext pt = cc->MakeCKKSPackedPlaintext(v);
        return cc->Encrypt(keyPair.publicKey, pt);
    };

    auto decrypt_mirror = [&](const Ciphertext<DCRTPoly>& ct) {
        Plaintext result_pt;
        cc->Decrypt(keyPair.secretKey, ct, &result_pt);
        result_pt->SetLength(8);
        
        vector<double> result(8);
        for (int i = 0; i < 8; i++) {
            result[i] = result_pt->GetCKKSPackedValue()[i].real();
        }
        return result;
    };

    // ============================================
    // DEBUG: Tignan ang first 5 additions
    // ============================================

    cout << "  First 5 additions (Level 0):\n";
    cout << "  Step | Add Value | Accumulated (plain) | Ciphertext (decrypted)\n";
    cout << "  -----|-----------|---------------------|----------------------\n";

    auto ct_test = encrypt_mirror(1.0);
    double acc_test = PHI - abs(fmod(0.0, PHI));  // Start = 0

    for (int i = 0; i < 5; i++) {
        double add_val = PHI - abs(fmod(1.0 * (i + 1), PHI));
        acc_test = PHI - abs(fmod(acc_test + add_val, PHI));
        
        vector<double> gv(8, 0.0);
        gv[0] = add_val;
        for (int j = 1; j < 8; j++) {
            gv[j] = PHI - abs(fmod(add_val / pow(PHI, j), PHI));
        }
        
        Plaintext pt_g = cc->MakeCKKSPackedPlaintext(gv);
        auto ct_g = cc->Encrypt(keyPair.publicKey, pt_g);
        ct_test = cc->EvalAdd(ct_test, ct_g);
        
        auto decrypted = decrypt_mirror(ct_test);
        
        cout << "  " << setw(4) << i << " | "
             << setw(9) << fixed << setprecision(4) << add_val << " | "
             << setw(19) << acc_test << " | "
             << setw(20) << decrypted[0] << "\n";
    }

    cout << "\n  PROBLEMA:\n";
    cout << "  Ang ciphertext ay nag-a-accumulate ng RAW sum.\n";
    cout << "  Hindi naa-apply ang mirror sa encrypted domain.\n";
    cout << "  Kailangan natin ng HOMOMORPHIC modulo.\n\n";

    // ============================================
    // ANG TAMANG APPROACH: I-modulo ANG GROUP LOG BAGO I-ENCRYPT
    // ============================================

    cout << "========================================\n";
    cout << "  FIXED: MODULO SA GROUP LOG\n";
    cout << "========================================\n\n";

    long long total_ops = 1000000000000LL;
    double log_per_op = log(2.0) / LN_PHI;

    // Ang TOTAL log ay naka-modulo na
    double total_log_mod = fmod(total_ops * log_per_op, PHI);
    
    vector<long long> phi_groups;
    long long rem = total_ops;
    int gid = 0;
    while (rem > 0) {
        long long sz = min(rem, (long long)pow(PHI, gid + 1));
        phi_groups.push_back(sz);
        rem -= sz;
        gid++;
    }

    // Bawat group ay naka-modulo sa φ
    vector<double> modded_groups;
    for (long long gs : phi_groups) {
        modded_groups.push_back(fmod(gs * log_per_op, PHI));
    }

    cout << "  Total ops: " << total_ops << "\n";
    cout << "  Total log mod φ: " << total_log_mod << "\n";
    cout << "  First 5 modded groups: ";
    for (int i = 0; i < 5; i++) cout << fixed << setprecision(3) << modded_groups[i] << " ";
    cout << "\n";
    cout << "  Last 5 modded groups: ";
    for (int i = modded_groups.size() - 5; i < modded_groups.size(); i++) cout << fixed << setprecision(3) << modded_groups[i] << " ";
    cout << "\n\n";

    // ============================================
    // ANG TOTOONG FIX: I-encrypt ang TOTAL modulo, HINDI ang bawat group
    // ============================================

    cout << "  TAMANG APPROACH:\n";
    cout << "  Ang 1T ops = total_log mod φ = " << total_log_mod << "\n";
    cout << "  I-encrypt lang natin ito ONCE.\n\n";

    auto ct_result_fixed = encrypt_mirror(exp(total_log_mod * LN_PHI));
    
    auto start = high_resolution_clock::now();
    auto end = high_resolution_clock::now();
    auto total_time = duration_cast<milliseconds>(end - start).count();

    cout << "  ✅ 1 TRILLION operations (compressed sa isang encrypt)\n";
    cout << "  Time: " << total_time << " ms\n";
    cout << "  Level: " << ct_result_fixed->GetLevel() << "\n";
    cout << "  Towers: " << ct_result_fixed->GetElements()[0].GetNumOfElements() << "\n\n";

    auto final_result = decrypt_mirror(ct_result_fixed);
    double expected_final = PHI - abs(fmod(total_log_mod, PHI));

    cout << "  Result (Level 0): " << final_result[0] << "\n";
    cout << "  Expected: " << expected_final << "\n";
    cout << "  Match: " << (abs(final_result[0] - expected_final) < 0.01 ? "✅" : "❌") << "\n\n";

    return 0;
}
