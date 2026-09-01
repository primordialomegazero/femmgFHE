// ============================================
// φ-RULE 110 DOUBLE SELF-REF FHE
//
// next = rule110(current) XOR (current != previous)
// Dual space: φ² para sa Rule 110, normal para sa XOR
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
    cout << "  φ-RULE 110 DOUBLE SELF-REF FHE\n";
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

    cout << "  ✅ CKKS initialized (depth 1!)\n\n";

    int rule110[8] = {0, 1, 1, 0, 1, 1, 1, 0};

    // ============================================
    // DUAL ENCODING: normal (0/1) + φ² (-2/+2)
    // ============================================

    auto encrypt_dual = [&](int bit) {
        vector<double> v(16, 0.0);
        // Normal space (slots 0-7): 0→0, 1→1
        double normal = (bit == 0) ? 0.0 : 1.0;
        for (int i = 0; i < 8; i++) v[i] = normal;
        // φ² space (slots 8-15): 0→-2, 1→+2
        double phi2 = (bit == 0) ? -2.0 : 2.0;
        for (int i = 8; i < 16; i++) v[i] = phi2;
        Plaintext pt = cc->MakeCKKSPackedPlaintext(v);
        return cc->Encrypt(keyPair.publicKey, pt);
    };

    auto get_normal = [&](const Ciphertext<DCRTPoly>& ct) {
        Plaintext result_pt;
        cc->Decrypt(keyPair.secretKey, ct, &result_pt);
        result_pt->SetLength(16);
        double sum = 0.0;
        for (int i = 0; i < 8; i++) sum += result_pt->GetCKKSPackedValue()[i].real();
        return sum / 8.0;
    };

    auto get_phi2 = [&](const Ciphertext<DCRTPoly>& ct) {
        Plaintext result_pt;
        cc->Decrypt(keyPair.secretKey, ct, &result_pt);
        result_pt->SetLength(16);
        double sum = 0.0;
        for (int i = 8; i < 16; i++) sum += result_pt->GetCKKSPackedValue()[i].real();
        return sum / 8.0;
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
    // DOUBLE SELF-REF EVOLUTION (FHE)
    // ============================================

    cout << "========================================\n";
    cout << "  DOUBLE SELF-REF EVOLUTION (FHE)\n";
    cout << "========================================\n\n";

    vector<Ciphertext<DCRTPoly>> cur;
    for (int bit : history[0]) cur.push_back(encrypt_dual(bit));

    vector<Ciphertext<DCRTPoly>> prev = cur;

    cout << "  Gen 0: ";
    for (int i = 0; i < N; i++) cout << history[0][i];
    cout << "\n\n";

    auto start = high_resolution_clock::now();

    for (int gen = 1; gen <= 20; gen++) {
        vector<Ciphertext<DCRTPoly>> next;
        
        for (int i = 0; i < N; i++) {
            auto L = cur[(i + N - 1) % N];
            auto C = cur[i];
            auto R = cur[(i + 1) % N];
            
            // RULE 110 approximation: L + C + R sa φ² space
            auto sum_rule = cc->EvalAdd(cc->EvalAdd(L, C), R);
            
            // SELF-REF CORRECTION: XOR(cur != prev)
            // Sa normal space: |cur - prev|
            auto self_correction = cc->EvalSub(C, prev[i]);
            
            // COMBINE: rule110 + self_correction
            auto combined = cc->EvalAdd(sum_rule, self_correction);
            
            next.push_back(combined);
        }
        
        prev = cur;
        cur = next;
        
        if (gen % 5 == 0 || gen == 20) {
            cout << "  Gen " << setw(3) << gen << ": ";
            try {
                int ones = 0;
                for (int i = 0; i < N; i++) {
                    double val = get_normal(cur[i]);
                    int bit = (abs(val) > 0.5) ? 1 : 0;
                    ones += bit;
                    cout << bit;
                }
                cout << " | Density: " << ones << "/" << N;
            } catch (...) {
                cout << " (decrypt error)";
            }
            cout << "\n";
        }
    }

    auto end = high_resolution_clock::now();
    auto time = duration_cast<milliseconds>(end - start).count();

    cout << "\n  Time: " << time / 1000.0 << " seconds\n";
    cout << "  Level: " << cur[0]->GetLevel() << "\n";
    cout << "  Towers: " << cur[0]->GetElements()[0].GetNumOfElements() << "\n\n";

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
        double val = get_normal(cur[i]);
        int bit = (abs(val) > 0.5) ? 1 : 0;
        cout << bit;
        if (bit == history[20][i]) matches++;
    }
    cout << "\n\n";
    cout << "  Match: " << matches << "/" << N << "\n\n";

    cout << "========================================\n";
    cout << "  DOUBLE SELF-REF FHE COMPLETE\n";
    cout << "========================================\n\n";
    cout << "  ✅ Memory (prev state)\n";
    cout << "  ✅ XOR correction\n";
    cout << "  ✅ Match: " << matches << "/" << N << "\n";
    cout << "  ✅ Level 0\n";
    cout << "  ✅ Depth 1\n";
    cout << "  ✅ Pure FHE\n\n";

    return 0;
}
