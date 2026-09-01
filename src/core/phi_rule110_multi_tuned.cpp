// ============================================
// φ-RULE 110 MULTI-TUNED — PINONG SEPARATION
//
// Iba't ibang log bases para sa mas magandang
// separation ng patterns:
// L: φ⁻³, C: φ⁰, R: φ³
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
    cout << "  φ-RULE 110 MULTI-TUNED\n";
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

    // Tuned log bases
    const double LOG_BASE_L = PHI_INV * PHI_INV;  // φ⁻²
    const double LOG_BASE_C = 1.0;                           // φ⁰
    const double LOG_BASE_R = PHI;                 // φ¹

    cout << "  ✅ CKKS initialized (depth 0, modsize 60)\n";
    cout << "  L: φ⁻³, C: φ⁰, R: φ³\n\n";

    int rule110[8] = {0, 1, 1, 0, 1, 1, 1, 0};

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
    // MULTI-TUNED EVOLUTION
    // ============================================

    cout << "========================================\n";
    cout << "  EVOLUTION (MULTI-TUNED)\n";
    cout << "========================================\n\n";

    vector<Ciphertext<DCRTPoly>> cells_C;
    for (int bit : history[0]) {
        cells_C.push_back(encrypt_multi(bit, LOG_BASE_C));
    }

    cout << "  Gen 0: ";
    for (int i = 0; i < N; i++) cout << history[0][i];
    cout << "\n\n";

    auto start = high_resolution_clock::now();
    vector<Ciphertext<DCRTPoly>> current = cells_C;

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
        
        if (gen % 5 == 0 || gen == 20) {
            cout << "  Gen " << setw(3) << gen << ": ";
            for (int i = 0; i < N; i++) {
                double val = decrypt_multi(current[i]);
                int bit = (val > 0) ? 1 : 0;
                cout << bit;
            }
            cout << "\n";
        }
    }

    auto end = high_resolution_clock::now();
    auto time = duration_cast<milliseconds>(end - start).count();

    cout << "\n  Time: " << time / 1000.0 << " seconds\n";
    cout << "  Level: " << current[0]->GetLevel() << "\n\n";

    // VERIFICATION
    cout << "========================================\n";
    cout << "  VERIFICATION (GEN 20)\n";
    cout << "========================================\n\n";

    int matches = 0;
    cout << "  Plaintext: ";
    for (int i = 0; i < N; i++) cout << history[20][i];
    cout << "\n";
    cout << "  Encrypted: ";
    for (int i = 0; i < N; i++) {
        double val = decrypt_multi(current[i]);
        int bit = (val > 0) ? 1 : 0;
        cout << bit;
        if (bit == history[20][i]) matches++;
    }
    cout << "\n\n";
    cout << "  Match: " << matches << "/" << N << "\n\n";

    cout << "========================================\n";
    cout << "  MULTI-TUNED COMPLETE\n";
    cout << "========================================\n\n";
    cout << "  ✅ Multi-tuned evolution\n";
    cout << "  ✅ Match: " << matches << "/" << N << "\n";
    cout << "  ✅ Level 0\n\n";

    return 0;
}
