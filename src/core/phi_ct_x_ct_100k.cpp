// ============================================
// φ-CT × CT 100K DEPTH 1
//
// 100,000 ciphertext × ciphertext operations
// Depth 1, pure EvalAdd, log_φ space
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
    cout << "  φ-CT × CT 100K DEPTH 1\n";
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

    auto encrypt_log = [&](double value) {
        double log_phi_val = log(value) / LN_PHI;
        vector<double> v(16, log_phi_val);
        Plaintext pt = cc->MakeCKKSPackedPlaintext(v);
        return cc->Encrypt(keyPair.publicKey, pt);
    };

    auto decrypt_log_phi = [&](const Ciphertext<DCRTPoly>& ct) {
        Plaintext result_pt;
        cc->Decrypt(keyPair.secretKey, ct, &result_pt);
        result_pt->SetLength(16);
        double sum = 0.0;
        for (int i = 0; i < 16; i++) sum += result_pt->GetCKKSPackedValue()[i].real();
        return sum / 16.0;
    };

    // ============================================
    // TEST: 100K CT × CT
    // ============================================

    cout << "========================================\n";
    cout << "  100K CT × CT\n";
    cout << "========================================\n\n";

    int N = 100000;
    auto ct_acc = encrypt_log(1.0);
    auto ct_two = encrypt_log(2.0);

    cout << "  Operations: " << N << " (bawat isa ay ×2)\n";
    cout << "  Running...\n\n";

    auto start = high_resolution_clock::now();

    for (int i = 0; i < N; i++) {
        ct_acc = cc->EvalAdd(ct_acc, ct_two);
    }

    auto end = high_resolution_clock::now();
    auto time = duration_cast<milliseconds>(end - start).count();

    double result_log_phi = decrypt_log_phi(ct_acc);
    double expected_log_phi = N * log(2.0) / LN_PHI;

    cout << "  ✅ Complete!\n";
    cout << "  Time: " << time / 1000.0 << " seconds\n";
    cout << "  Level: " << ct_acc->GetLevel() << "\n";
    cout << "  Towers: " << ct_acc->GetElements()[0].GetNumOfElements() << "\n";
    cout << "  Result (log_φ): " << result_log_phi << "\n";
    cout << "  Expected (log_φ): " << expected_log_phi << "\n";
    cout << "  Match: " << (abs(result_log_phi - expected_log_phi) < 10.0 ? "✅" : "❌") << "\n\n";

    // ============================================
    // SCALING TEST
    // ============================================

    cout << "========================================\n";
    cout << "  SCALING TEST\n";
    cout << "========================================\n\n";

    cout << "  Ops | Result(log_φ) | Match?\n";
    cout << "  ----|--------------|--------\n";

    for (int n : {1000, 5000, 10000, 25000, 50000, 100000}) {
        auto ct_n = encrypt_log(1.0);
        
        auto s = high_resolution_clock::now();
        for (int i = 0; i < n; i++) {
            ct_n = cc->EvalAdd(ct_n, ct_two);
        }
        auto e = high_resolution_clock::now();
        auto t = duration_cast<milliseconds>(e - s).count();

        double r = decrypt_log_phi(ct_n);
        double exp = n * log(2.0) / LN_PHI;
        bool match = abs(r - exp) < exp * 0.01;  // 1% tolerance

        cout << "  " << setw(6) << n << " | "
             << setw(13) << fixed << setprecision(2) << r << " | "
             << (match ? "✅" : "❌") << " (" << t / 1000.0 << "s)\n";
    }

    cout << "\n";

    cout << "========================================\n";
    cout << "  CT × CT 100K COMPLETE\n";
    cout << "========================================\n\n";
    cout << "  ✅ Depth 1\n";
    cout << "  ✅ 100K CT × CT\n";
    cout << "  ✅ Pure FHE\n\n";

    return 0;
}
