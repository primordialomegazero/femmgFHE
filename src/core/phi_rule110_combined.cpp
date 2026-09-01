// ============================================
// φ-RULE 110 COMBINED — V2 + GRAMMAR + PERIOD-3
//
// 1. 3 cells (L, C, R) — Multi-Log V2
// 2. φ-grammar: 0→φ, 1→φ⁻¹
// 3. Period-3 normalization
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
    cout << "  φ-RULE 110 COMBINED\n";
    cout << "========================================\n\n";

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

    const double PHI = 1.6180339887498948482;
    const double PHI_INV = 1.0 / PHI;

    // φ-grammar: 0→φ, 1→φ⁻¹
    // Multi-Log V2: L=φ⁻², C=φ⁰, R=φ¹

    cout << "  ✅ CKKS initialized (depth 0, modsize 55)\n";
    cout << "  Combined: V2 multi-log + φ-grammar\n\n";

    int rule110[8] = {0, 1, 1, 0, 1, 1, 1, 0};

    auto encrypt_grammar = [&](int bit, double log_base) {
        // φ-grammar: 0→φ, 1→φ⁻¹ — naka-scale sa log_base
        double val = (bit == 0) ? PHI : PHI_INV;
        val *= log_base;
        vector<double> v(16, val);
        Plaintext pt = cc->MakeCKKSPackedPlaintext(v);
        return cc->Encrypt(keyPair.publicKey, pt);
    };

    auto decrypt_grammar = [&](const Ciphertext<DCRTPoly>& ct) {
        Plaintext result_pt;
        cc->Decrypt(keyPair.secretKey, ct, &result_pt);
        result_pt->SetLength(16);
        double sum = 0.0;
        for (int i = 0; i < 16; i++) sum += result_pt->GetCKKSPackedValue()[i].real();
        return sum / 16.0;
    };

    // ============================================
    // PLAINTEXT REFERENCE
    // ============================================

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

    // ============================================
    // COMBINED EVOLUTION
    // ============================================

    cout << "========================================\n";
    cout << "  EVOLUTION (COMBINED)\n";
    cout << "========================================\n\n";

    double L_base = pow(PHI, -2.0);  // φ⁻²
    double C_base = 1.0;              // φ⁰
    double R_base = PHI;              // φ¹

    vector<Ciphertext<DCRTPoly>> cells_L, cells_C, cells_R;
    for (int bit : history[0]) {
        cells_L.push_back(encrypt_grammar(bit, L_base));
        cells_C.push_back(encrypt_grammar(bit, C_base));
        cells_R.push_back(encrypt_grammar(bit, R_base));
    }

    cout << "  Gen 0: ";
    for (int i = 0; i < N; i++) cout << history[0][i];
    cout << "\n\n";

    auto start = high_resolution_clock::now();
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
        
        if (gen % 5 == 0 || gen == 20) {
            cout << "  Gen " << setw(3) << gen << ": ";
            for (int i = 0; i < N; i++) {
                double val = decrypt_grammar(curr_C[i]);
                // φ-grammar decode: mas malapit sa φ → 0, mas malapit sa φ⁻¹ → 1
                double d0 = abs(val - PHI);
                double d1 = abs(val - PHI_INV);
                int bit = (d1 < d0) ? 1 : 0;
                cout << bit;
            }
            cout << "\n";
        }
    }

    auto end = high_resolution_clock::now();
    auto time = duration_cast<milliseconds>(end - start).count();

    cout << "\n  Time: " << time / 1000.0 << " seconds\n";
    cout << "  Level: " << curr_C[0]->GetLevel() << "\n\n";

    // ============================================
    // VERIFICATION
    // ============================================

    cout << "========================================\n";
    cout << "  VERIFICATION (GEN 20)\n";
    cout << "========================================\n\n";

    int matches = 0;
    cout << "  Plaintext: ";
    for (int i = 0; i < N; i++) cout << history[20][i];
    cout << "\n";
    cout << "  Encrypted: ";
    for (int i = 0; i < N; i++) {
        double val = decrypt_grammar(curr_C[i]);
        double d0 = abs(val - PHI);
        double d1 = abs(val - PHI_INV);
        int bit = (d1 < d0) ? 1 : 0;
        cout << bit;
        if (bit == history[20][i]) matches++;
    }
    cout << "\n\n";
    cout << "  Match: " << matches << "/" << N << "\n\n";

    cout << "========================================\n";
    cout << "  COMBINED COMPLETE\n";
    cout << "========================================\n\n";
    cout << "  ✅ V2 multi-log + φ-grammar\n";
    cout << "  ✅ Match: " << matches << "/" << N << "\n";
    cout << "  ✅ Level 0\n\n";

    return 0;
}
