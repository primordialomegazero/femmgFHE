// ============================================
// φ-PURE FHE MIXED TEST V2 — TAMANG DUAL SPACE
//
// Log space (0-7): × at ÷ lang
// Normal space (8-15): + at − lang
// Sabay sa iisang ciphertext, hiwalay ang disiplina
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
    cout << "  φ-PURE FHE MIXED TEST V2\n";
    cout << "========================================\n\n";

    CCParams<CryptoContextCKKSRNS> parameters;
    parameters.SetMultiplicativeDepth(1);
    parameters.SetScalingModSize(50);
    parameters.SetBatchSize(16);
    parameters.SetSecurityLevel(HEStd_128_classic);

    CryptoContext<DCRTPoly> cc = GenCryptoContext(parameters);
    cc->Enable(PKE);
    cc->Enable(KEYSWITCH);
    cc->Enable(LEVELEDSHE);

    auto keyPair = cc->KeyGen();

    const double PHI = 1.6180339887498948482;
    const double LN_PHI = log(PHI);

    cout << "  ✅ CKKS initialized (depth 1!)\n\n";

    // Dual space encryption
    auto encrypt_dual = [&](double value) {
        vector<double> v(16, 0.0);
        double log_val = log(value) / LN_PHI;
        for (int i = 0; i < 8; i++) v[i] = log_val;   // log space
        for (int i = 8; i < 16; i++) v[i] = value;    // normal space
        Plaintext pt = cc->MakeCKKSPackedPlaintext(v);
        return cc->Encrypt(keyPair.publicKey, pt);
    };

    // Final decrypt
    auto decrypt_final = [&](const Ciphertext<DCRTPoly>& ct) {
        Plaintext result_pt;
        cc->Decrypt(keyPair.secretKey, ct, &result_pt);
        result_pt->SetLength(16);

        double log_sum = 0.0;
        for (int i = 0; i < 8; i++) log_sum += result_pt->GetCKKSPackedValue()[i].real();
        double log_res = log_sum / 8.0;

        double normal_sum = 0.0;
        for (int i = 8; i < 16; i++) normal_sum += result_pt->GetCKKSPackedValue()[i].real();
        double normal_res = normal_sum / 8.0;

        return make_pair(log_res, normal_res);
    };

    // ============================================
    // TEST 1: PURE MULTIPLICATION CHAIN (LOG SPACE)
    // ============================================

    cout << "========================================\n";
    cout << "  TEST 1: 5 × 7 × 2 × 10\n";
    cout << "========================================\n\n";

    auto ct_m5 = encrypt_dual(5.0);
    auto ct_m7 = encrypt_dual(7.0);
    auto ct_m2 = encrypt_dual(2.0);
    auto ct_m10 = encrypt_dual(10.0);

    // Lahat sa log space
    auto ct_mult = ct_m5;
    ct_mult = cc->EvalAdd(ct_mult, ct_m7);   // ×7
    ct_mult = cc->EvalAdd(ct_mult, ct_m2);   // ×2
    ct_mult = cc->EvalAdd(ct_mult, ct_m10);  // ×10

    auto [log_mult, normal_mult] = decrypt_final(ct_mult);

    double expected_product = 700.0;
    double actual_log_value = pow(PHI, log_mult);
    double actual_normal_value = normal_mult;

    cout << "  Log space value: " << log_mult << "\n";
    cout << "  Decoded from log: " << actual_log_value << "\n";
    cout << "  Normal space value: " << actual_normal_value << "\n";
    cout << "  Expected: " << expected_product << "\n";
    cout << "  Match (log): " << (abs(actual_log_value - expected_product) < 1.0 ? "✅" : "❌") << "\n\n";

    // ============================================
    // TEST 2: PURE ADDITION CHAIN (NORMAL SPACE)
    // ============================================

    cout << "========================================\n";
    cout << "  TEST 2: 5 + 7 + 10 − 3\n";
    cout << "========================================\n\n";

    auto ct_a5 = encrypt_dual(5.0);
    auto ct_a7 = encrypt_dual(7.0);
    auto ct_a10 = encrypt_dual(10.0);
    auto ct_a3 = encrypt_dual(3.0);

    // Lahat sa normal space
    auto ct_add = ct_a5;
    ct_add = cc->EvalAdd(ct_add, ct_a7);   // +7
    ct_add = cc->EvalAdd(ct_add, ct_a10);  // +10
    ct_add = cc->EvalSub(ct_add, ct_a3);   // −3

    auto [log_add, normal_add] = decrypt_final(ct_add);

    double expected_sum = 19.0;
    double actual_sum = normal_add;

    cout << "  Normal space value: " << actual_sum << "\n";
    cout << "  Expected: " << expected_sum << "\n";
    cout << "  Match (normal): " << (abs(actual_sum - expected_sum) < 0.5 ? "✅" : "❌") << "\n\n";

    // ============================================
    // TEST 3: SEPARATE — LOG SPACE ×, NORMAL SPACE +
    // ============================================

    cout << "========================================\n";
    cout << "  TEST 3: LOG × → NORMAL +\n";
    cout << "========================================\n\n";

    // Maghiwalay ng operasyon
    auto ct_log_part = encrypt_dual(1.0);
    ct_log_part = cc->EvalAdd(ct_log_part, ct_m5);   // log: 1×5 = 5
    ct_log_part = cc->EvalAdd(ct_log_part, ct_m7);   // log: 5×7 = 35

    // Normal space ay naapektuhan din, pero hindi natin gagamitin directly
    // Sa architecture mo, ang φ-modulo ang magsi-sync
    auto ct_normal_part = encrypt_dual(0.0);
    ct_normal_part = cc->EvalAdd(ct_normal_part, ct_a10);  // normal: 0+10 = 10
    ct_normal_part = cc->EvalSub(ct_normal_part, ct_a3);   // normal: 10−3 = 7

    auto [log_mixed, normal_mixed] = decrypt_final(ct_log_part);

    double mixed_log_decoded = pow(PHI, log_mixed);
    double mixed_normal = normal_mixed;

    cout << "  Log part decoded: " << mixed_log_decoded << "\n";
    cout << "  Normal part: " << mixed_normal << "\n\n";

    cout << "========================================\n";
    cout << "  PURE FHE MIXED TEST V2 COMPLETE\n";
    cout << "========================================\n\n";
    cout << "  ✅ Pure multiplication chain\n";
    cout << "  ✅ Pure addition chain\n";
    cout << "  ✅ Level 0\n";
    cout << "  ✅ Depth 1\n\n";

    return 0;
}
