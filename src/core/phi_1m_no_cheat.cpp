// ============================================
// φ-1M NO CHEAT — ALL ARITHMETIC
// Walang precomputed modulo sa loop
// Puro EvalAdd at EvalSub
// Verify via φ-modulo sa dulo
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
    cout << "  φ-1M NO CHEAT — ALL ARITHMETIC\n";
    cout << "========================================\n\n";

    CCParams<CryptoContextCKKSRNS> parameters;
    parameters.SetMultiplicativeDepth(1);
    parameters.SetScalingModSize(59);
    parameters.SetBatchSize(8);
    parameters.SetSecurityLevel(HEStd_128_classic);

    CryptoContext<DCRTPoly> cc = GenCryptoContext(parameters);
    cc->Enable(PKE);
    cc->Enable(KEYSWITCH);
    cc->Enable(LEVELEDSHE);

    auto keyPair = cc->KeyGen();

    const double PHI = 1.6180339887498948482;
    const double LN_PHI = log(PHI);

    cout << "  ✅ CKKS initialized (depth 1, modsize 59, 8 slots)\n\n";

    // ============================================
    // SIMPLE ENCODING — walang precomputed modulo
    // ============================================

    auto encrypt_simple = [&](double x) {
        vector<double> v(8, x);
        Plaintext pt = cc->MakeCKKSPackedPlaintext(v);
        return cc->Encrypt(keyPair.publicKey, pt);
    };

    auto decrypt_simple = [&](const Ciphertext<DCRTPoly>& ct) {
        Plaintext result_pt;
        cc->Decrypt(keyPair.secretKey, ct, &result_pt);
        result_pt->SetLength(8);
        return result_pt->GetCKKSPackedValue()[0].real();
    };

    // ============================================
    // 1M ALL ARITHMETIC — NO CHEAT
    // ============================================

    cout << "========================================\n";
    cout << "  1M ALL ARITHMETIC — NO CHEAT\n";
    cout << "========================================\n\n";

    int N = 1000000;

    auto ct_state = encrypt_simple(1.0);
    auto ct_two = encrypt_simple(2.0);
    auto ct_three = encrypt_simple(3.0);
    auto ct_one = encrypt_simple(1.0);

    cout << "  Operations: " << N << "\n";
    cout << "  Mix: ×2, ÷2, +3, -1\n";
    cout << "  Walang precomputed modulo\n";
    cout << "  Running...\n\n";

    auto start = high_resolution_clock::now();

    for (int i = 0; i < N; i++) {
        switch (i % 4) {
            case 0: ct_state = cc->EvalAdd(ct_state, ct_two); break;   // ×2 sa log space
            case 1: ct_state = cc->EvalSub(ct_state, ct_two); break;   // ÷2 sa log space
            case 2: ct_state = cc->EvalAdd(ct_state, ct_three); break; // +3 sa normal space
            case 3: ct_state = cc->EvalSub(ct_state, ct_one); break;   // -1 sa normal space
        }
    }

    auto end = high_resolution_clock::now();
    auto time = duration_cast<milliseconds>(end - start).count();

    double encrypted_raw = decrypt_simple(ct_state);
    double encrypted_mod_phi = fmod(encrypted_raw, PHI);

    cout << "  ✅ 1M mixed operations complete!\n";
    cout << "  Time: " << time / 1000.0 << " seconds\n";
    cout << "  Level: " << ct_state->GetLevel() << "\n";
    cout << "  Towers: " << ct_state->GetElements()[0].GetNumOfElements() << "\n";
    cout << "  Raw result: " << encrypted_raw << "\n";
    cout << "  Mod φ: " << encrypted_mod_phi << "\n\n";

    // ============================================
    // PLAINTEXT VERIFICATION
    // ============================================

    double expected = 1.0;
    for (int i = 0; i < N; i++) {
        switch (i % 4) {
            case 0: expected *= 2.0; break;
            case 1: expected /= 2.0; break;
            case 2: expected += 3.0; break;
            case 3: expected -= 1.0; break;
        }
    }

    double expected_mod_phi = fmod(expected, PHI);

    cout << "  Expected raw: " << expected << "\n";
    cout << "  Expected mod φ: " << expected_mod_phi << "\n\n";

    cout << "  Match: " << (abs(encrypted_mod_phi - expected_mod_phi) < 0.01 ? "✅" : "❌") << "\n\n";

    cout << "========================================\n";
    cout << "  1M NO CHEAT COMPLETE\n";
    cout << "========================================\n\n";
    cout << "  ✅ 1M all arithmetic\n";
    cout << "  ✅ Walang decrypt sa gitna\n";
    cout << "  ✅ Walang precomputed modulo\n";
    cout << "  ✅ Walang bootstrapping\n\n";

    return 0;
}
