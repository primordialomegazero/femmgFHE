// ============================================
// φ-MIXED ARITHMETIC TEST
// Subok kung ang dual space ay kayang gawin ang
// (a × b) + c nang walang decrypt
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
    cout << "  φ-MIXED ARITHMETIC TEST\n";
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

    // Decrypt final
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
    // TEST: (5 × 7) + 3 = 38
    // ============================================

    cout << "========================================\n";
    cout << "  TEST: (5 × 7) + 3\n";
    cout << "========================================\n\n";

    auto ct_5 = encrypt_dual(5.0);
    auto ct_7 = encrypt_dual(7.0);
    auto ct_3 = encrypt_dual(3.0);

    // Step 1: 5 × 7 sa log space
    auto ct_mult = cc->EvalAdd(ct_5, ct_7);

    // Step 2: + 3 sa normal space
    // Subok: EvalAdd lang ba sapat?
    auto ct_result = cc->EvalAdd(ct_mult, ct_3);

    auto [log_final, normal_final] = decrypt_final(ct_result);

    cout << "  Log space final: " << log_final << "\n";
    cout << "  Decoded from log: " << pow(PHI, log_final) << "\n";
    cout << "  Normal space final: " << normal_final << "\n";
    cout << "  Expected: 38\n";
    cout << "  Match (normal): " << (abs(normal_final - 38.0) < 0.5 ? "✅" : "❌") << "\n\n";

    cout << "  Level: " << ct_result->GetLevel() << "\n";
    cout << "  Towers: " << ct_result->GetElements()[0].GetNumOfElements() << "\n\n";

    return 0;
}
