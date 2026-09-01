// ============================================
// φ-RULE 110 SWEEP — HANAPIN ANG BEST COMBO
//
// I-test ang iba't ibang φ-power combinations
// para sa L, C, R log bases
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
    cout << "  φ-RULE 110 SWEEP — BEST COMBO\n";
    cout << "========================================\n\n";

    const double PHI = 1.6180339887498948482;
    const double PHI_INV = 1.0 / PHI;

    int rule110[8] = {0, 1, 1, 0, 1, 1, 1, 0};

    // Mga susubukan na powers para sa L, C, R
    vector<double> powers = {-3, -2.5, -2, -1.5, -1, -0.5, 0, 0.5, 1, 1.5, 2};

    cout << "  L_power | C_power | R_power | Match\n";
    cout << "  --------|---------|---------|------\n";

    int best_match = 0;
    double best_L = 0, best_C = 0, best_R = 0;

    // Simplified: test L at R powers, C ay 0
    for (double L_pow : {-3.0, -2.0, -1.0, -0.5}) {
        for (double R_pow : {0.5, 1.0, 1.5, 2.0}) {
            CCParams<CryptoContextCKKSRNS> parameters;
            parameters.SetMultiplicativeDepth(0);
            parameters.SetScalingModSize(55);
            parameters.SetBatchSize(16);
            parameters.SetSecurityLevel(HEStd_128_classic);

            CryptoContext<DCRTPoly> cc = GenCryptoContext(parameters);
            cc->Enable(PKE);
            cc->Enable(KEYSWITCH);
            cc->Enable(LEVELEDSHE);

            auto keyPair = cc->KeyGen();

            double L_base = pow(PHI, L_pow);
            double C_base = 1.0;
            double R_base = pow(PHI, R_pow);

            auto encrypt_multi = [&](int bit, double log_base) {
                double val = (bit == 0) ? -log_base : log_base;
                vector<double> v(16, val);
                Plaintext pt = cc->MakeCKKSPackedPlaintext(v);
                return cc->Encrypt(keyPair.publicKey, pt);
            };

            auto decrypt_multi = [&](const Ciphertext<DCRTPoly>& ct) {
                Plaintext result_pt;
                cc->Decrypt(keyPair.secretKey, ct, &result_pt);
                result_pt->SetLength(16);
                double sum = 0.0;
                for (int i = 0; i < 16; i++) sum += result_pt->GetCKKSPackedValue()[i].real();
                return sum / 16.0;
            };

            int N = 16;
            vector<int> plain(N, 0);
            plain[7] = 1;
            plain[8] = 1;

            vector<vector<int>> history;
            history.push_back(plain);
            for (int gen = 0; gen < 20; gen++) {
                vector<int> next(N, 0);
                for (int i = 0; i < N; i++) {
                    int L = plain[(i + N - 1) % N];
                    int C = plain[i];
                    int R = plain[(i + 1) % N];
                    int pattern = (L << 2) | (C << 1) | R;
                    next[i] = rule110[pattern];
                }
                plain = next;
                history.push_back(plain);
            }

            vector<Ciphertext<DCRTPoly>> cells;
            for (int bit : history[0]) {
                cells.push_back(encrypt_multi(bit, C_base));
            }

            vector<Ciphertext<DCRTPoly>> current = cells;

            for (int gen = 1; gen <= 20; gen++) {
                vector<Ciphertext<DCRTPoly>> next;
                for (int i = 0; i < N; i++) {
                    auto L = current[(i + N - 1) % N];
                    auto C = current[i];
                    auto R = current[(i + 1) % N];
                    auto sum1 = cc->EvalAdd(L, C);
                    auto sum2 = cc->EvalAdd(sum1, R);
                    next.push_back(sum2);
                }
                current = next;
            }

            int matches = 0;
            for (int i = 0; i < N; i++) {
                double val = decrypt_multi(current[i]);
                int bit = (val > 0) ? 1 : 0;
                if (bit == history[20][i]) matches++;
            }

            cout << "  " << setw(6) << L_pow << " | "
                 << setw(7) << 0 << " | "
                 << setw(7) << R_pow << " | "
                 << setw(5) << matches << "/16";

            if (matches > best_match) {
                best_match = matches;
                best_L = L_pow;
                best_C = 0;
                best_R = R_pow;
                cout << " ⭐";
            }
            cout << "\n";
        }
    }

    cout << "\n";
    cout << "  BEST: L=φ^" << best_L << ", C=φ^" << best_C << ", R=φ^" << best_R;
    cout << " → " << best_match << "/16\n\n";

    cout << "========================================\n";
    cout << "  SWEEP COMPLETE\n";
    cout << "========================================\n\n";

    return 0;
}
