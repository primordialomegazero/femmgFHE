// ============================================
// φ-RULE 110 FINE-TUNE — 14/16 → 16/16
//
// Fine-tune sa paligid ng L=φ⁻³, R=φ¹·²⁵
// para makuha ang 16/16
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
    cout << "  φ-RULE 110 FINE-TUNE\n";
    cout << "========================================\n\n";

    const double PHI = 1.6180339887498948482;

    int rule110[8] = {0, 1, 1, 0, 1, 1, 1, 0};

    // Fine-tune sa paligid ng best: L=-3.0, R=1.25
    vector<double> L_powers = {-3.2, -3.1, -3.0, -2.9, -2.8};
    vector<double> R_powers = {1.1, 1.15, 1.2, 1.25, 1.3, 1.35};

    cout << "  L_power | R_power | Match\n";
    cout << "  --------|---------|------\n";

    int best_match = 0;
    double best_L = -3.0, best_R = 1.25;

    for (double L_pow : L_powers) {
        for (double R_pow : R_powers) {
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

            auto encrypt_fine = [&](int bit, double base) {
                double val = (bit == 0) ? -pow(PHI, base) : pow(PHI, base);
                vector<double> v(16, val);
                Plaintext pt = cc->MakeCKKSPackedPlaintext(v);
                return cc->Encrypt(keyPair.publicKey, pt);
            };

            auto decrypt_fine = [&](const Ciphertext<DCRTPoly>& ct) {
                Plaintext result_pt;
                cc->Decrypt(keyPair.secretKey, ct, &result_pt);
                result_pt->SetLength(16);
                double sum = 0.0;
                for (int i = 0; i < 16; i++) sum += result_pt->GetCKKSPackedValue()[i].real();
                return sum / 16.0;
            };

            // Plaintext reference
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

            // Evolution
            vector<Ciphertext<DCRTPoly>> cells_L, cells_C, cells_R;
            for (int bit : history[0]) {
                cells_L.push_back(encrypt_fine(bit, L_pow));
                cells_C.push_back(encrypt_fine(bit, 0.0));
                cells_R.push_back(encrypt_fine(bit, R_pow));
            }

            vector<Ciphertext<DCRTPoly>> curr_L = cells_L, curr_C = cells_C, curr_R = cells_R;

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
                double val = decrypt_fine(curr_C[i]);
                int bit = (val > 0) ? 1 : 0;
                if (bit == history[20][i]) matches++;
            }

            cout << "  " << setw(7) << fixed << setprecision(2) << L_pow << " | "
                 << setw(7) << R_pow << " | "
                 << setw(5) << matches << "/16";
            
            if (matches > best_match) {
                best_match = matches;
                best_L = L_pow;
                best_R = R_pow;
                cout << " ⭐";
            }
            cout << "\n";
        }
    }

    cout << "\n  BEST: L=φ^" << best_L << ", C=φ^0, R=φ^" << best_R << " → " << best_match << "/16\n\n";

    return 0;
}
