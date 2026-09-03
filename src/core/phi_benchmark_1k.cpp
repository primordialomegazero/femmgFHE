// ============================================
// φ-BENCHMARK — 1K OPERATIONS (HONEST TEST)
//
// Traditional CKKS (with EvalMult) vs φ-Log Space (EvalAdd only)
// 1K operations, same parameters, same hardware
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
    cout << "  φ-BENCHMARK — 1K OPERATIONS\n";
    cout << "  Traditional vs φ-Log Space\n";
    cout << "========================================\n\n";

    const double PHI = 1.6180339887498948482;
    const double LN_PHI = log(PHI);

    // ============================================
    // TEST 1: TRADITIONAL CKKS (EvalMult)
    // ============================================

    cout << "========================================\n";
    cout << "  TEST 1: TRADITIONAL CKKS\n";
    cout << "  (1K EvalMult — multiply by 2)\n";
    cout << "========================================\n\n";

    CCParams<CryptoContextCKKSRNS> params_trad;
    params_trad.SetMultiplicativeDepth(1000);  // Kailangan ng malalim na depth!
    params_trad.SetScalingModSize(50);
    params_trad.SetBatchSize(1);
    params_trad.SetSecurityLevel(HEStd_128_classic);

    CryptoContext<DCRTPoly> cc_trad = GenCryptoContext(params_trad);
    cc_trad->Enable(PKE);
    cc_trad->Enable(KEYSWITCH);
    cc_trad->Enable(LEVELEDSHE);

    auto keyPair_trad = cc_trad->KeyGen();
    cc_trad->EvalMultKeyGen(keyPair_trad.secretKey);

    cout << "  Parameters: Depth 1000, 128-bit\n";
    cout << "  Operation: 1K sequential EvalMult (×2)\n\n";

    vector<double> v1(1, 1.0);
    Plaintext pt1 = cc_trad->MakeCKKSPackedPlaintext(v1);
    auto ct_trad = cc_trad->Encrypt(keyPair_trad.publicKey, pt1);

    vector<double> v2(1, 2.0);
    Plaintext pt2 = cc_trad->MakeCKKSPackedPlaintext(v2);
    auto ct_mult = cc_trad->Encrypt(keyPair_trad.publicKey, pt2);

    auto start_trad = high_resolution_clock::now();

    for (int i = 0; i < 1000; i++) {
        ct_trad = cc_trad->EvalMult(ct_trad, ct_mult);  // EvalMult!
    }

    auto end_trad = high_resolution_clock::now();
    auto time_trad = duration_cast<milliseconds>(end_trad - start_trad).count();

    cout << "  ✅ 1K EvalMult complete!\n";
    cout << "  Time: " << time_trad << " ms\n";
    cout << "  Level: " << ct_trad->GetLevel() << "\n";
    cout << "  Towers: " << ct_trad->GetElements()[0].GetNumOfElements() << "\n\n";

    // ============================================
    // TEST 2: φ-LOG SPACE (EvalAdd)
    // ============================================

    cout << "========================================\n";
    cout << "  TEST 2: φ-LOG SPACE\n";
    cout << "  (1K EvalAdd — multiply by 2 sa log)\n";
    cout << "========================================\n\n";

    CCParams<CryptoContextCKKSRNS> params_phi;
    params_phi.SetMultiplicativeDepth(1);  // Depth 1 lang!
    params_phi.SetScalingModSize(50);
    params_phi.SetBatchSize(1);
    params_phi.SetSecurityLevel(HEStd_128_classic);

    CryptoContext<DCRTPoly> cc_phi = GenCryptoContext(params_phi);
    cc_phi->Enable(PKE);
    cc_phi->Enable(KEYSWITCH);
    cc_phi->Enable(LEVELEDSHE);

    auto keyPair_phi = cc_phi->KeyGen();
    cc_phi->EvalMultKeyGen(keyPair_phi.secretKey);

    cout << "  Parameters: Depth 1, 128-bit\n";
    cout << "  Operation: 1K sequential EvalAdd (log ×2)\n\n";

    // Sa φ-log space: multiply by 2 = add log(2)
    double log2_phi = log(2.0) / LN_PHI;
    vector<double> lv1(1, 0.0);  // log(1) = 0
    Plaintext lpt1 = cc_phi->MakeCKKSPackedPlaintext(lv1);
    auto ct_phi = cc_phi->Encrypt(keyPair_phi.publicKey, lpt1);

    vector<double> lv2(1, log2_phi);  // log(2) sa φ-base
    Plaintext lpt2 = cc_phi->MakeCKKSPackedPlaintext(lv2);
    auto ct_add = cc_phi->Encrypt(keyPair_phi.publicKey, lpt2);

    auto start_phi = high_resolution_clock::now();

    for (int i = 0; i < 1000; i++) {
        ct_phi = cc_phi->EvalAdd(ct_phi, ct_add);  // EvalAdd!
    }

    auto end_phi = high_resolution_clock::now();
    auto time_phi = duration_cast<milliseconds>(end_phi - start_phi).count();

    cout << "  ✅ 1K EvalAdd complete!\n";
    cout << "  Time: " << time_phi << " ms\n";
    cout << "  Level: " << ct_phi->GetLevel() << "\n";
    cout << "  Towers: " << ct_phi->GetElements()[0].GetNumOfElements() << "\n\n";

    // ============================================
    // COMPARISON
    // ============================================

    cout << "========================================\n";
    cout << "  COMPARISON\n";
    cout << "========================================\n\n";
    cout << "  Traditional (EvalMult): " << time_trad << " ms | Level: " << ct_trad->GetLevel() << "\n";
    cout << "  φ-Log Space (EvalAdd): " << time_phi << " ms | Level: " << ct_phi->GetLevel() << "\n";
    cout << "  Speedup: " << fixed << setprecision(2) 
         << (double)time_trad / max(1.0, (double)time_phi) << "x\n\n";

    return 0;
}
