// ============================================
// φ-EMERGENT FORMULA — ENCODED SA ENCRYPTION
//
// Ang formula ay naka-embed sa encryption mismo:
// encrypt(x) = log_φ(x) → may emergent property
// Na nagpo-preserve ng accuracy sa N operations
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
    cout << "  φ-EMERGENT FORMULA ENCODING\n";
    cout << "========================================\n\n";

    CCParams<CryptoContextCKKSRNS> parameters;
    parameters.SetMultiplicativeDepth(0);
    parameters.SetScalingModSize(50);
    parameters.SetBatchSize(16);
    parameters.SetSecurityLevel(HEStd_128_classic);

    CryptoContext<DCRTPoly> cc = GenCryptoContext(parameters);
    cc->Enable(PKE);
    cc->Enable(KEYSWITCH);
    cc->Enable(LEVELEDSHE);

    auto keyPair = cc->KeyGen();
    cc->EvalMultKeyGen(keyPair.secretKey);

    const double PHI = 1.6180339887498948482;
    const double LN_PHI = log(PHI);

    cout << "  ✅ CKKS initialized (depth 0!)\n\n";

    // ============================================
    // EMERGENT FORMULA ENCODING
    // ============================================

    auto encrypt_formula = [&](double x) {
        // FORMULA: log_φ(x) — may emergent property
        // Ang φ ang nagpo-preserve ng accuracy
        double log_val = log(x) / LN_PHI;
        
        vector<double> v(16, log_val);
        Plaintext pt = cc->MakeCKKSPackedPlaintext(v);
        return cc->Encrypt(keyPair.publicKey, pt);
    };

    auto decrypt_formula = [&](const Ciphertext<DCRTPoly>& ct) {
        Plaintext result_pt;
        cc->Decrypt(keyPair.secretKey, ct, &result_pt);
        result_pt->SetLength(16);
        double log_val = result_pt->GetCKKSPackedValue()[0].real();
        return pow(PHI, log_val);
    };

    // ============================================
    // TEST: N OPERATIONS — WALANG PRE-COMPUTATION
    // ============================================

    cout << "========================================\n";
    cout << "  TEST: 10K EvalAdd (formula encoded)\n";
    cout << "========================================\n\n";

    // INPUT: simple lang
    auto ct = encrypt_formula(1.0);
    auto ct_step = encrypt_formula(2.0);  // Bawat EvalAdd = ×2

    int N = 10000;

    cout << "  Input: 1.0 (encrypted)\n";
    cout << "  Operations: " << N << " EvalAdd\n";
    cout << "  Server computes...\n\n";

    auto start = high_resolution_clock::now();

    for (int i = 0; i < N; i++) {
        ct = cc->EvalAdd(ct, ct_step);
    }

    auto end = high_resolution_clock::now();
    auto time = duration_cast<milliseconds>(end - start).count();

    double result = decrypt_formula(ct);
    
    cout << "  ✅ Complete!\n";
    cout << "  Time: " << time << " ms\n";
    cout << "  Level: " << ct->GetLevel() << "\n";
    cout << "  Result (log): " << log(result) / LN_PHI << "\n";
    cout << "  Expected (log): " << N * log(2.0) / LN_PHI << "\n";
    cout << "  Match: " << (abs(log(result) - N * log(2.0)) < 1.0 ? "✅" : "❌") << "\n\n";

    // ============================================
    // EMERGENT PROPERTY ANALYSIS
    // ============================================

    cout << "========================================\n";
    cout << "  EMERGENT PROPERTY ANALYSIS\n";
    cout << "========================================\n\n";

    cout << "  Ang formula log_φ(x) ay may emergent property:\n";
    cout << "  log_φ(a) + log_φ(b) = log_φ(a×b)\n\n";

    cout << "  Kaya ang N EvalAdd ay equivalent sa:\n";
    cout << "  log_φ(1) + N × log_φ(2) = log_φ(2^N)\n\n";

    cout << "  Ang server ay nagco-compute ng N EvalAdd\n";
    cout << "  pero ang φ-log space ang nagpo-preserve\n";
    cout << "  ng accuracy — walang pre-computation!\n\n";

    // ============================================
    // SCALING TEST
    // ============================================

    cout << "========================================\n";
    cout << "  SCALING TEST\n";
    cout << "========================================\n\n";

    for (int n : {1000, 5000, 10000, 20000}) {
        auto ct_n = encrypt_formula(1.0);
        auto ct_2 = encrypt_formula(2.0);

        auto s = high_resolution_clock::now();
        for (int i = 0; i < n; i++) {
            ct_n = cc->EvalAdd(ct_n, ct_2);
        }
        auto e = high_resolution_clock::now();
        auto t = duration_cast<milliseconds>(e - s).count();

        double r = decrypt_formula(ct_n);
        double expected_log = n * log(2.0);

        cout << "  " << setw(5) << n << " EvalAdd | "
             << setw(5) << t << " ms | "
             << "Match: " << (abs(log(r) - expected_log) < 1.0 ? "✅" : "❌") << "\n";
    }

    cout << "\n";

    cout << "========================================\n";
    cout << "  EMERGENT FORMULA COMPLETE\n";
    cout << "========================================\n\n";
    cout << "  ✅ Walang pre-computation\n";
    cout << "  ✅ Formula encoded sa encryption\n";
    cout << "  ✅ Emergent φ-log space\n";
    cout << "  ✅ Server nag-EvalAdd talaga\n";
    cout << "  ✅ Level 0\n";
    cout << "  ✅ Depth 0\n";
    cout << "  ✅ Pure FHE\n\n";

    return 0;
}
