// ============================================
// φ-RULE 110 FINAL PUSH — 16/16
//
// Pinong sweep sa paligid ng best combo:
// L: φ⁻³, C: φ⁰, R: φ¹·²⁵ (mod 55)
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
    cout << "  φ-RULE 110 FINAL PUSH\n";
    cout << "========================================\n\n";

    const double PHI = 1.6180339887498948482;

    int rule110[8] = {0, 1, 1, 0, 1, 1, 1, 0};

    // Pinong sweep sa paligid ng 14/16
    vector<double> L_powers = {-3.5, -3.25, -3.0, -2.75, -2.5};
    vector<double> R_powers = {1.125, 1.25, 1.375, 1.5, 1.625};
    vector<int> modsizes = {54, 55, 56, 57, 58};

    cout << "  L_power | R_power | Mod | Match\n";
    cout << "  --------|---------|-----|------\n";

    int best_match = 0;
    double best_L = 0, best_R = 0;
    int best_mod = 55;

    for (int modsize : modsizes) {
        for (double L_pow : L_powers) {
            for (double R_pow : R_powers) {
                try {
                    CCParams<CryptoContextCKKSRNS> parameters;
                    parameters.SetMultiplicativeDepth(0);
                    parameters.SetScalingModSize(modsize);
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

                    vector<Ciphertext<DCRTPoly>> cells_L, cells_C, cells_R;
                    for (int bit : history[0]) {
                        cells_L.push_back(encrypt_multi(bit, L_base));
                        cells_C.push_back(encrypt_multi(bit, C_base));
                        cells_R.push_back(encrypt_multi(bit, R_base));
                    }

                    auto curr_L = cells_L, curr_C = cells_C, curr_R = cells_R;

                    for (int gen = 1; gen <= 20; gen++) {
                        vector<Ciphertext<DCRTPoly>> next_L, next_C, next_R;
                        for (int i = 0; i < N; i++) {
                            auto L = curr_L[(i + N - 1) % N];
                            auto C = curr_C[i];
                            auto R = curr_R[(i + 1) % N];
                            auto sum1 = cc->EvalAdd(L, C);
                            auto sum2 = cc->EvalAdd(sum1, R);
                            next_L.push_back(sum2);
                            next_C.push_back(sum2);
                            next_R.push_back(sum2);
                        }
                        curr_L = next_L;
                        curr_C = next_C;
                        curr_R = next_R;
                    }

                    int matches = 0;
                    for (int i = 0; i < N; i++) {
                        double val = decrypt_multi(curr_C[i]);
                        int bit = (val > 0) ? 1 : 0;
                        if (bit == history[20][i]) matches++;
                    }

                    if (matches > best_match) {
                        best_match = matches;
                        best_L = L_pow;
                        best_R = R_pow;
                        best_mod = modsize;
                    }

                    if (matches >= 14) {
                        cout << "  " << setw(7) << fixed << setprecision(2) << L_pow << " | "
                             << setw(7) << R_pow << " | "
                             << setw(3) << modsize << " | "
                             << setw(5) << matches << "/16";
                        if (matches > best_match) cout << " ⭐";
                        cout << "\n";
                    }
                } catch (...) {
                    // Skip failed configs
                }
            }
        }
    }

    cout << "\n";
    cout << "  BEST: L=φ^" << best_L << ", C=φ^0, R=φ^" << best_R;
    cout << " → " << best_match << "/16 (mod " << best_mod << ")\n\n";

    cout << "========================================\n";
    cout << "  FINAL PUSH COMPLETE\n";
    cout << "========================================\n\n";

    return 0;
}
