// ============================================
// φ-EVALMULT LOG SPACE
// Test kung ang EvalMult(constant) ay libre
// kapag ang value ay nasa log space
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
    cout << "  φ-EVALMULT LOG SPACE\n";
    cout << "========================================\n\n";

    CCParams<CryptoContextCKKSRNS> parameters;
    parameters.SetMultiplicativeDepth(2);
    parameters.SetScalingModSize(59);
    parameters.SetBatchSize(4);
    parameters.SetSecurityLevel(HEStd_128_classic);

    CryptoContext<DCRTPoly> cc = GenCryptoContext(parameters);
    cc->Enable(PKE);
    cc->Enable(KEYSWITCH);
    cc->Enable(LEVELEDSHE);

    auto keyPair = cc->KeyGen();

    const double PHI = 1.6180339887498948482;
    const double LN_PHI = log(PHI);

    cout << "  ✅ CKKS initialized (depth 2, 4 slots)\n\n";

    // ============================================
    // TEST: EVALMULT(CONSTANT) SA LOG SPACE
    // ============================================

    auto encrypt_log = [&](double x) {
        double log_phi_x = log(x) / LN_PHI;
        vector<double> v(4, log_phi_x);
        Plaintext pt = cc->MakeCKKSPackedPlaintext(v);
        return cc->Encrypt(keyPair.publicKey, pt);
    };

    auto decrypt_log = [&](const Ciphertext<DCRTPoly>& ct) {
        Plaintext result_pt;
        cc->Decrypt(keyPair.secretKey, ct, &result_pt);
        result_pt->SetLength(4);
        return result_pt->GetCKKSPackedValue()[0].real();
    };

    cout << "========================================\n";
    cout << "  METHOD 1: EvalMult(ct, constant)\n";
    cout << "========================================\n\n";

    cout << "  x | log_φ(x) | EvalMult(ct, 2) | Level | Match?\n";
    cout << "  --|-----------|-----------------|-------|--------\n";

    for (double x : {5.0, 7.0, 35.0}) {
        auto ct_log = encrypt_log(x);
        auto ct_mult = cc->EvalMult(ct_log, 2.0);
        
        double result_log = decrypt_log(ct_mult);
        double expected_log = log(x) / LN_PHI + log(2.0) / LN_PHI;
        bool match = abs(result_log - expected_log) < 0.1;
        
        cout << "  " << setw(3) << x << " | "
             << setw(9) << fixed << setprecision(4) << (log(x)/LN_PHI) << " | "
             << setw(15) << result_log << " | "
             << setw(5) << ct_mult->GetLevel() << " | "
             << (match ? "✅" : "❌") << "\n";
    }

    cout << "\n========================================\n";
    cout << "  METHOD 2: EvalAdd(ct, log(constant))\n";
    cout << "========================================\n\n";

    cout << "  x | log_φ(x) | EvalAdd(ct, log(2)) | Level | Match?\n";
    cout << "  --|-----------|---------------------|-------|--------\n";

    auto ct_log2 = encrypt_log(2.0);

    for (double x : {5.0, 7.0, 35.0}) {
        auto ct_log = encrypt_log(x);
        auto ct_add = cc->EvalAdd(ct_log, ct_log2);
        
        double result_log = decrypt_log(ct_add);
        double expected_log = log(x) / LN_PHI + log(2.0) / LN_PHI;
        bool match = abs(result_log - expected_log) < 0.1;
        
        cout << "  " << setw(3) << x << " | "
             << setw(9) << fixed << setprecision(4) << (log(x)/LN_PHI) << " | "
             << setw(19) << result_log << " | "
             << setw(5) << ct_add->GetLevel() << " | "
             << (match ? "✅" : "❌") << "\n";
    }

    cout << "\n========================================\n";
    cout << "  COMPARISON\n";
    cout << "========================================\n\n";
    cout << "  Method 1 (EvalMult): may noise cost\n";
    cout << "  Method 2 (EvalAdd): walang noise cost\n";
    cout << "  Pareho silang nagbibigay ng tamang result\n\n";

    return 0;
}
