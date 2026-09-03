// ============================================
// φ-DEPTH SCALING FIXED
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
    cout << "  φ-DEPTH SCALING FIXED\n";
    cout << "========================================\n\n";

    const double PHI = 1.6180339887498948482;
    const double LN_PHI = log(PHI);

    vector<int> depths = {0, 1, 2, 3, 5, 10};
    vector<int> scales = {10, 100, 1000, 10000, 100000, 1000000};

    cout << "  Scale vs Depth Matrix:\n";
    cout << "  ======================\n\n";

    cout << "  Ops |";
    for (int d : depths) cout << " D" << setw(2) << d << " |";
    cout << "\n  ----|";
    for (size_t i = 0; i < depths.size(); i++) cout << "-----|";
    cout << "\n";

    for (int N : scales) {
        cout << "  " << setw(4) << N << " |";
        
        for (int depth : depths) {
            try {
                CCParams<CryptoContextCKKSRNS> parameters;
                parameters.SetMultiplicativeDepth(depth);
                parameters.SetScalingModSize(50);
                parameters.SetBatchSize(16);
                parameters.SetSecurityLevel(HEStd_128_classic);

                CryptoContext<DCRTPoly> cc = GenCryptoContext(parameters);
                cc->Enable(PKE);
                cc->Enable(KEYSWITCH);
                cc->Enable(LEVELEDSHE);

                auto keyPair = cc->KeyGen();
                cc->EvalMultKeyGen(keyPair.secretKey);

                auto encrypt_ct = [&](double value) {
                    double log_val = log(value) / LN_PHI;  // log_φ space
                    vector<double> v(16, log_val);
                    Plaintext pt = cc->MakeCKKSPackedPlaintext(v);
                    return cc->Encrypt(keyPair.publicKey, pt);
                };

                auto decrypt_log = [&](const Ciphertext<DCRTPoly>& ct) {
                    Plaintext result_pt;
                    cc->Decrypt(keyPair.secretKey, ct, &result_pt);
                    result_pt->SetLength(16);
                    double sum = 0.0;
                    for (int i = 0; i < 16; i++) sum += result_pt->GetCKKSPackedValue()[i].real();
                    return sum / 16.0;  // log_φ space
                };

                auto ct = encrypt_ct(1.0);
                auto ct_two = encrypt_ct(2.0);

                for (int i = 0; i < N; i++) {
                    ct = cc->EvalAdd(ct, ct_two);
                }

                double result = decrypt_log(ct);  // log_φ(2^N)
                double expected = N * log(2.0) / LN_PHI;  // N × log_φ(2)
                double error = abs(result - expected);

                // 5% tolerance
                if (error < abs(expected) * 0.05) {
                    cout << "  ✅  |";
                } else {
                    cout << "  ❌  |";
                }
            } catch (...) {
                cout << "  💥  |";
            }
        }
        cout << "\n";
    }

    cout << "\n";

    // ============================================
    // TIME TEST PARA SA 1M
    // ============================================

    cout << "========================================\n";
    cout << "  TIME: 1M OPS — ANONG DEPTH?\n";
    cout << "========================================\n\n";

    for (int depth : {0, 1, 2, 3, 5, 10}) {
        CCParams<CryptoContextCKKSRNS> parameters;
        parameters.SetMultiplicativeDepth(depth);
        parameters.SetScalingModSize(50);
        parameters.SetBatchSize(16);
        parameters.SetSecurityLevel(HEStd_128_classic);

        CryptoContext<DCRTPoly> cc = GenCryptoContext(parameters);
        cc->Enable(PKE);
        cc->Enable(KEYSWITCH);
        cc->Enable(LEVELEDSHE);

        auto keyPair = cc->KeyGen();
        cc->EvalMultKeyGen(keyPair.secretKey);

        auto encrypt_ct = [&](double value) {
            double log_val = log(value) / LN_PHI;
            vector<double> v(16, log_val);
            Plaintext pt = cc->MakeCKKSPackedPlaintext(v);
            return cc->Encrypt(keyPair.publicKey, pt);
        };

        auto decrypt_log = [&](const Ciphertext<DCRTPoly>& ct) {
            Plaintext result_pt;
            cc->Decrypt(keyPair.secretKey, ct, &result_pt);
            result_pt->SetLength(16);
            double sum = 0.0;
            for (int i = 0; i < 16; i++) sum += result_pt->GetCKKSPackedValue()[i].real();
            return sum / 16.0;
        };

        auto ct = encrypt_ct(1.0);
        auto ct_two = encrypt_ct(2.0);

        auto start = high_resolution_clock::now();
        for (int i = 0; i < 1000000; i++) {
            ct = cc->EvalAdd(ct, ct_two);
        }
        auto end = high_resolution_clock::now();
        auto time = duration_cast<milliseconds>(end - start).count();

        double result = decrypt_log(ct);
        double expected = 1000000 * log(2.0) / LN_PHI;
        double error = abs(result - expected) / abs(expected) * 100.0;

        cout << "  Depth " << setw(2) << depth << ": "
             << "Time: " << setw(7) << time << " ms | "
             << "Error: " << fixed << setprecision(2) << error << "% | "
             << (error < 5.0 ? "✅" : "❌") << "\n";
    }

    cout << "\n";

    cout << "========================================\n";
    cout << "  SUMMARY\n";
    cout << "========================================\n\n";
    cout << "  Mas malaking depth = mas malaking precision\n";
    cout << "  Pero mas mabagal din!\n\n";

    return 0;
}
