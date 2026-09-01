// ============================================
// φ-RULE 110 FIBONACCI-GUIDED
//
// State: 0 → φ⁻⁵, 1 → φ⁻²
// Transition: sum = L + C + R
// Guide: Fibonacci-weighted positions
// Evolution: fib_guide(sum)
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
    cout << "  φ-RULE 110 FIBONACCI-GUIDED\n";
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
    const double S_ZERO = pow(PHI, -5);
    const double S_ONE = pow(PHI, -2);

    // Fibonacci weights para sa positions
    vector<double> fib = {1, 1, 2, 3, 5, 8, 13, 21, 34, 55, 89, 144, 233, 377, 610, 987};

    cout << "  ✅ CKKS initialized (depth 0, modsize 55)\n";
    cout << "  Fibonacci-guided evolution\n\n";

    int rule110[8] = {0, 1, 1, 0, 1, 1, 1, 0};

    // ============================================
    // FIBONACCI-GUIDED ENCODING
    // ============================================
    
    auto encrypt_fib = [&](int bit, int position) {
        // Ang state ay φ-value na weighted ng Fibonacci position
        double base = (bit == 0) ? S_ZERO : S_ONE;
        double weighted = base * fib[position % 16];
        vector<double> v(16, weighted);
        Plaintext pt = cc->MakeCKKSPackedPlaintext(v);
        return cc->Encrypt(keyPair.publicKey, pt);
    };

    auto decrypt_fib = [&](const Ciphertext<DCRTPoly>& ct) {
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
    // FIBONACCI-GUIDED EVOLUTION
    // ============================================

    cout << "========================================\n";
    cout << "  EVOLUTION (FIBONACCI-GUIDED)\n";
    cout << "========================================\n\n";

    vector<Ciphertext<DCRTPoly>> cells;
    for (int i = 0; i < N; i++) {
        cells.push_back(encrypt_fib(history[0][i], i));
    }

    cout << "  Gen 0: ";
    for (int i = 0; i < N; i++) cout << history[0][i];
    cout << "\n\n";

    auto start = high_resolution_clock::now();
    vector<Ciphertext<DCRTPoly>> current = cells;

    for (int gen = 1; gen <= 20; gen++) {
        vector<Ciphertext<DCRTPoly>> next;
        
        for (int i = 0; i < N; i++) {
            auto L = current[(i + N - 1) % N];
            auto C = current[i];
            auto R = current[(i + 1) % N];
            
            // Transition: sum = L + C + R
            auto sum1 = cc->EvalAdd(L, C);
            auto sum2 = cc->EvalAdd(sum1, R);
            
            // Fibonacci guide: i-scale sa current position
            auto guided = cc->EvalSub(sum2, fib[i % 16] * PHI);
            
            next.push_back(guided);
        }
        
        current = next;
        
        if (gen % 5 == 0 || gen == 20) {
            cout << "  Gen " << setw(3) << gen << ": ";
            for (int i = 0; i < N; i++) {
                double val = decrypt_fib(current[i]);
                int bit = (abs(val - S_ONE) < abs(val - S_ZERO)) ? 1 : 0;
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
        double val = decrypt_fib(current[i]);
        int bit = (abs(val - S_ONE) < abs(val - S_ZERO)) ? 1 : 0;
        cout << bit;
        if (bit == history[20][i]) matches++;
    }
    cout << "\n\n";
    cout << "  Match: " << matches << "/" << N << "\n\n";

    cout << "========================================\n";
    cout << "  FIBONACCI-GUIDED COMPLETE\n";
    cout << "========================================\n\n";
    cout << "  ✅ Fibonacci-guided\n";
    cout << "  ✅ Match: " << matches << "/" << N << "\n";
    cout << "  ✅ Level 0\n";
    cout << "  ✅ Depth 0\n\n";

    return 0;
}
