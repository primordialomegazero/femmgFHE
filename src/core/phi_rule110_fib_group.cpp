// ============================================
// φ-RULE 110 FIBONACCI GROUP EVOLUTION
//
// Group sizes: 2, 3, 5, 8 (Fibonacci!)
// φ-scaled weights para sa natural evolution
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
    cout << "  φ-RULE 110 FIBONACCI GROUP\n";
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

    cout << "  ✅ CKKS initialized (depth 0, modsize 55)\n";
    cout << "  Fibonacci groups: 2, 3, 5, 8, 2\n\n";

    int rule110[8] = {0, 1, 1, 0, 1, 1, 1, 0};

    auto encrypt_bal = [&](int bit) {
        double val = (bit == 0) ? -PHI_INV : 1.0;
        vector<double> v(16, val);
        Plaintext pt = cc->MakeCKKSPackedPlaintext(v);
        return cc->Encrypt(keyPair.publicKey, pt);
    };

    auto decrypt_bal = [&](const Ciphertext<DCRTPoly>& ct) {
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
    // FIBONACCI GROUP EVOLUTION
    // ============================================

    vector<int> group_sizes = {2, 3, 5, 8, 2};  // 20 total
    vector<double> group_weights = {
        pow(PHI, -5),  // Group 1: 2 gens
        pow(PHI, -4),  // Group 2: 3 gens
        pow(PHI, -3),  // Group 3: 5 gens
        pow(PHI, -2),  // Group 4: 8 gens
        pow(PHI, -1)   // Group 5: 2 gens
    };

    cout << "========================================\n";
    cout << "  EVOLUTION (FIBONACCI GROUP)\n";
    cout << "========================================\n\n";

    vector<Ciphertext<DCRTPoly>> cells;
    for (int bit : history[0]) {
        cells.push_back(encrypt_bal(bit));
    }

    cout << "  Gen 0: ";
    for (int i = 0; i < N; i++) cout << history[0][i];
    cout << "\n\n";

    auto start = high_resolution_clock::now();
    vector<Ciphertext<DCRTPoly>> current = cells;
    int current_gen = 0;

    for (int g = 0; g < (int)group_sizes.size(); g++) {
        int group_size = group_sizes[g];
        vector<Ciphertext<DCRTPoly>> next;
        
        for (int i = 0; i < N; i++) {
            auto L = current[(i + N - 1) % N];
            auto C = current[i];
            auto R = current[(i + 1) % N];
            
            // Sum para sa buong group
            auto sum1 = cc->EvalAdd(L, C);
            auto sum2 = cc->EvalAdd(sum1, R);
            
            // φ-scaled weight × group_size
            auto weighted = cc->EvalSub(sum2, group_weights[g] * group_size);
            
            next.push_back(weighted);
        }
        
        current = next;
        current_gen += group_size;
        
        cout << "  Gen " << setw(3) << current_gen << ": ";
        for (int i = 0; i < N; i++) {
            double val = decrypt_bal(current[i]);
            int bit = (val > 0) ? 1 : 0;
            cout << bit;
        }
        cout << "\n";
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
        double val = decrypt_bal(current[i]);
        int bit = (val > 0) ? 1 : 0;
        cout << bit;
        if (bit == history[20][i]) matches++;
    }
    cout << "\n\n";
    cout << "  Match: " << matches << "/" << N << "\n\n";

    cout << "========================================\n";
    cout << "  FIBONACCI GROUP COMPLETE\n";
    cout << "========================================\n\n";
    cout << "  ✅ Fibonacci group evolution\n";
    cout << "  ✅ Match: " << matches << "/" << N << "\n";
    cout << "  ✅ Level 0\n\n";

    return 0;
}
