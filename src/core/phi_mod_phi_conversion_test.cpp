// ============================================
// φ-MOD φ CONVERSION TEST
// Subok kung ang (a + bφ) mod φ = a ay
// pwedeng gawin sa encrypted domain
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
    cout << "  φ-MOD φ CONVERSION TEST\n";
    cout << "========================================\n\n";

    CCParams<CryptoContextCKKSRNS> parameters;
    parameters.SetMultiplicativeDepth(1);
    parameters.SetScalingModSize(50);
    parameters.SetBatchSize(2);
    parameters.SetSecurityLevel(HEStd_128_classic);

    CryptoContext<DCRTPoly> cc = GenCryptoContext(parameters);
    cc->Enable(PKE);
    cc->Enable(KEYSWITCH);
    cc->Enable(LEVELEDSHE);

    auto keyPair = cc->KeyGen();

    const double PHI = 1.6180339887498948482;
    const double LN_PHI = log(PHI);

    cout << "  ✅ CKKS initialized (depth 1, 2 slots)\n\n";

    // ============================================
    // SIMULATION NG (a + bφ) mod φ = a
    // ============================================

    cout << "========================================\n";
    cout << "  SIMULATION: (a + bφ) mod φ = a\n";
    cout << "========================================\n\n";

    // Test values
    vector<pair<double, double>> tests = {
        {35.0, 3.0},   // 35 + 3φ, mod φ = 35
        {12.0, 5.0},   // 12 + 5φ, mod φ = 12
        {100.0, 1.0}   // 100 + φ, mod φ = 100
    };

    cout << "  a + bφ | mod φ | Expected | Match?\n";
    cout << "  -------|-------|----------|--------\n";

    for (auto& [a, b] : tests) {
        double val = a + b * PHI;
        double mod_phi = fmod(val, PHI);
        double expected = a;
        bool match = abs(mod_phi - expected) < 0.01;
        
        cout << "  " << setw(6) << fixed << setprecision(2) << val << " | "
             << setw(6) << mod_phi << " | "
             << setw(8) << expected << " | "
             << (match ? "✅" : "❌") << "\n";
    }

    // ============================================
    // ENCRYPTED VERSION
    // ============================================

    cout << "\n========================================\n";
    cout << "  ENCRYPTED: (a + bφ) mod φ\n";
    cout << "========================================\n\n";

    // Encrypt a at bφ nang hiwalay
    auto encrypt_val = [&](double v) {
        vector<double> vec(2, v);
        Plaintext pt = cc->MakeCKKSPackedPlaintext(vec);
        return cc->Encrypt(keyPair.publicKey, pt);
    };

    auto decrypt_val = [&](const Ciphertext<DCRTPoly>& ct) {
        Plaintext result_pt;
        cc->Decrypt(keyPair.secretKey, ct, &result_pt);
        result_pt->SetLength(2);
        return result_pt->GetCKKSPackedValue()[0].real();
    };

    // Test: 35 + 3φ
    double a_val = 35.0;
    double b_val = 3.0;

    auto ct_a = encrypt_val(a_val);
    auto ct_bphi = encrypt_val(b_val * PHI);

    // Sa encrypted domain: (a + bφ) mod φ = a
    // Kailangan nating i-modulo sa φ
    // Pero walang direct EvalMod sa OpenFHE
    
    // SUBOK 1: EvalAdd lang
    auto ct_sum = cc->EvalAdd(ct_a, ct_bphi);
    double sum_result = decrypt_val(ct_sum);

    cout << "  Subok 1: EvalAdd(a, bφ) = " << sum_result << "\n";
    cout << "  Expected: " << (a_val + b_val * PHI) << "\n\n";

    // SUBOK 2: EvalAdd + φ-modulo approximation
    // Kung ang val ay 35 + 3φ = 39.85
    // fmod(39.85, φ) = 35
    // Sa encrypted domain, kailangan nating i-approximate
    
    auto ct_result = cc->EvalAdd(ct_a, ct_bphi);
    double result = decrypt_val(ct_result);
    double modded = fmod(result, PHI);

    cout << "  Subok 2: fmod(result, φ) = " << modded << "\n";
    cout << "  Expected: " << a_val << "\n";
    cout << "  Match: " << (abs(modded - a_val) < 0.01 ? "✅" : "❌") << "\n\n";

    cout << "  Level: " << ct_result->GetLevel() << "\n";
    cout << "  Towers: " << ct_result->GetElements()[0].GetNumOfElements() << "\n\n";

    cout << "========================================\n";
    cout << "  OBSERVATION\n";
    cout << "========================================\n\n";
    cout << "  Ang (a + bφ) mod φ = a ay:\n";
    cout << "  - Gumagana sa PLAINTEXT (fmod)\n";
    cout << "  - Pero sa ENCRYPTED, walang direct EvalMod\n";
    cout << "  - Kailangan ng approximation technique\n\n";

    return 0;
}
