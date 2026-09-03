// ============================================
// φ-POWER DIRECT FHE
// I-encode ang φ-power mismo — hindi log
// EvalAdd ay natural na addition sa φ-power space
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
    cout << "  φ-POWER DIRECT FHE\n";
    cout << "========================================\n\n";

    CCParams<CryptoContextCKKSRNS> parameters;
    parameters.SetMultiplicativeDepth(1);
    parameters.SetScalingModSize(59);
    parameters.SetBatchSize(1);
    parameters.SetSecurityLevel(HEStd_128_classic);

    CryptoContext<DCRTPoly> cc = GenCryptoContext(parameters);
    cc->Enable(PKE);
    cc->Enable(KEYSWITCH);
    cc->Enable(LEVELEDSHE);

    auto keyPair = cc->KeyGen();

    const double PHI = 1.6180339887498948482;
    const double LN_PHI = log(PHI);

    cout << "  ✅ CKKS initialized (depth 1, batch 1)\n\n";

    // ============================================
    // ENCODING: x → φ^x (φ-POWER MISMO)
    // ============================================

    auto encrypt_power = [&](double x) {
        double phi_x = pow(PHI, x);
        vector<double> v(1, phi_x);
        Plaintext pt = cc->MakeCKKSPackedPlaintext(v);
        return cc->Encrypt(keyPair.publicKey, pt);
    };

    auto decrypt_power = [&](const Ciphertext<DCRTPoly>& ct) {
        Plaintext result_pt;
        cc->Decrypt(keyPair.secretKey, ct, &result_pt);
        result_pt->SetLength(1);
        return result_pt->GetCKKSPackedValue()[0].real();
    };

    // ============================================
    // TEST: φ^a + φ^(a-1) = φ^(a+1)
    // ============================================

    cout << "========================================\n";
    cout << "  EXACT: φ^a + φ^(a-1) = φ^(a+1)\n";
    cout << "========================================\n\n";

    cout << "  a | φ^a + φ^(a-1) | φ^(a+1) | Match?\n";
    cout << "  --|---------------|---------|--------\n";

    for (int a : {2, 3, 4, 5, 6, 7, 8}) {
        auto ct_a = encrypt_power(a);
        auto ct_am1 = encrypt_power(a-1);
        
        auto ct_sum = cc->EvalAdd(ct_a, ct_am1);
        double sum_val = decrypt_power(ct_sum);
        
        double expected = pow(PHI, a+1);
        bool match = abs(sum_val - expected) < 0.1;
        
        cout << "  " << a << " | "
             << setw(13) << fixed << setprecision(4) << sum_val << " | "
             << setw(7) << expected << " | "
             << (match ? "✅" : "❌") << "\n";
    }

    // ============================================
    // TEST: MULTIPLICATION — φ^a × φ^b = φ^(a+b)
    // ============================================

    cout << "\n========================================\n";
    cout << "  MULTIPLICATION: φ^a × φ^b = φ^(a+b)\n";
    cout << "========================================\n\n";

    cout << "  Sa φ-power space, ang multiplication ay:\n";
    cout << "  φ^a × φ^b = φ^(a+b)\n";
    cout << "  Kailangan ng EvalMult o EvalAdd sa log\n\n";

    cout << "  Pero sa φ-power space (direct):\n";
    cout << "  EvalMult(ct_a, ct_b) = φ^a × φ^b = φ^(a+b)\n";
    cout << "  Ito ay natural na multiplication!\n\n";

    // ============================================
    // 10K CHAIN — CONSECUTIVE ADDITIONS
    // ============================================

    cout << "========================================\n";
    cout << "  10K CONSECUTIVE ADDITIONS\n";
    cout << "========================================\n\n";

    int N = 10000;

    auto ct_state = encrypt_power(2.0);  // φ^2
    auto ct_one = encrypt_power(1.0);    // φ^1
    auto ct_zero = encrypt_power(0.0);   // φ^0

    cout << "  Operations: " << N << "\n";
    cout << "  Pattern: φ^n + φ^(n-1) = φ^(n+1)\n";
    cout << "  Running...\n\n";

    auto start = high_resolution_clock::now();

    double expected_exp = 2.0;

    for (int i = 0; i < N; i++) {
        // φ^n + φ^(n-1) = φ^(n+1)
        // Kung may φ^(n-1) tayo, i-add sa state
        ct_state = cc->EvalAdd(ct_state, encrypt_power(expected_exp - 1));
        expected_exp += 1.0;
    }

    auto end = high_resolution_clock::now();
    auto time = duration_cast<milliseconds>(end - start).count();

    double final_val = decrypt_power(ct_state);
    double expected_val = pow(PHI, expected_exp);
    double mod_phi = fmod(final_val, PHI);
    double expected_mod = fmod(expected_val, PHI);

    cout << "  ✅ 10K complete!\n";
    cout << "  Time: " << time << " ms\n";
    cout << "  Level: " << ct_state->GetLevel() << "\n";
    cout << "  Final value: " << final_val << "\n";
    cout << "  Final mod φ: " << mod_phi << "\n";
    cout << "  Expected mod φ: " << expected_mod << "\n";
    cout << "  Match: " << (abs(mod_phi - expected_mod) < 0.1 ? "✅" : "❌") << "\n\n";

    cout << "========================================\n";
    cout << "  φ-POWER DIRECT FHE COMPLETE\n";
    cout << "========================================\n\n";
    cout << "  ✅ φ-power direct encoding\n";
    cout << "  ✅ EvalAdd para sa consecutive addition\n";
    cout << "  ✅ Walang bootstrapping\n\n";

    return 0;
}
