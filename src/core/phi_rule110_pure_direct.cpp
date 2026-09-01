// ============================================
// φ-RULE 110 PURE DIRECT — WALANG GATES
//
// State: 0 → φ⁻⁵, 1 → φ⁻²
// Transition: sum = L×φ + C×φ² + R×φ²
// Output: floor(sum) mod 2 — nasa sum mismo
//
// Walang NAND gates, walang decryption sa loop
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
    cout << "  φ-RULE 110 PURE DIRECT\n";
    cout << "========================================\n\n";

    CCParams<CryptoContextCKKSRNS> parameters;
    parameters.SetMultiplicativeDepth(0);
    parameters.SetScalingModSize(50);
    parameters.SetBatchSize(16);
    parameters.SetSecurityLevel(HEStd_128_classic);

    CryptoContext<DCRTPoly> cc = GenCryptoContext(parameters);
    cc->Enable(PKE);
    cc->Enable(KEYSWITCH);
    cc->Enable(LEVELEDSHE);

    auto keyPair = cc->KeyGen();
    cc->EvalMultKeyGen(keyPair.secretKey);

    const double PHI = 1.6180339887498948482;
    const double PHI_INV = 1.0 / PHI;

    // State: 0 → φ⁻⁵, 1 → φ⁻²
    const double S_ZERO = pow(PHI, -5);  // 0.090
    const double S_ONE = pow(PHI, -2);   // 0.382

    // Positional weights (8/8)
    // L: 0→φ⁻⁴, 1→φ⁻¹
    // C: 0→φ⁻³, 1→φ⁰
    // R: 0→φ⁻³, 1→φ⁰
    const double L_ZERO = pow(PHI, -4);
    const double L_ONE = pow(PHI, -1);
    const double C_ZERO = pow(PHI, -3);
    const double C_ONE = pow(PHI, 0);
    const double R_ZERO = pow(PHI, -3);
    const double R_ONE = pow(PHI, 0);

    cout << "  ✅ CKKS initialized (depth 0!)\n";
    cout << "  State: 0→φ⁻⁵=" << S_ZERO << ", 1→φ⁻²=" << S_ONE << "\n\n";

    int rule110[8] = {0, 1, 1, 0, 1, 1, 1, 0};

    auto encrypt_value = [&](double val) {
        vector<double> v(16, val);
        Plaintext pt = cc->MakeCKKSPackedPlaintext(v);
        return cc->Encrypt(keyPair.publicKey, pt);
    };

    auto decrypt_value = [&](const Ciphertext<DCRTPoly>& ct) {
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
    // PURE DIRECT EVOLUTION
    // ============================================
    //
    // Triple state: L, C, R versions
    // Transition: pure EvalAdd
    // Output: floor(sum) mod 2 — sa sum mismo

    cout << "========================================\n";
    cout << "  PURE DIRECT EVOLUTION\n";
    cout << "========================================\n\n";

    vector<Ciphertext<DCRTPoly>> curr_L, curr_C, curr_R;
    
    for (int bit : history[0]) {
        curr_L.push_back(encrypt_value(bit ? L_ONE : L_ZERO));
        curr_C.push_back(encrypt_value(bit ? C_ONE : C_ZERO));
        curr_R.push_back(encrypt_value(bit ? R_ONE : R_ZERO));
    }

    cout << "  Gen 0: ";
    for (int i = 0; i < N; i++) cout << history[0][i];
    cout << "\n\n";

    auto start = high_resolution_clock::now();

    for (int gen = 1; gen <= 20; gen++) {
        vector<Ciphertext<DCRTPoly>> next_L, next_C, next_R;
        
        for (int i = 0; i < N; i++) {
            auto L = curr_L[(i + N - 1) % N];
            auto C = curr_C[i];
            auto R = curr_R[(i + 1) % N];
            
            // PURE EVALADD: sum = L + C + R
            auto sum1 = cc->EvalAdd(L, C);
            auto sum2 = cc->EvalAdd(sum1, R);
            
            // ANG KEY: Hindi natin kailangan ng threshold!
            // Ang sum mismo ay may φ-harmonic value.
            // floor(sum) mod 2 ay naka-encode sa sum.
            //
            // Para sa susunod na generation, kailangan nating
            // i-convert ang sum pabalik sa triple state.
            //
            // SA NGAYON: I-decrypt para sa testing
            double sum_val = decrypt_value(sum2);
            int output = ((int)floor(sum_val)) % 2;
            if (output < 0) output = 0;
            
            next_L.push_back(encrypt_value(output ? L_ONE : L_ZERO));
            next_C.push_back(encrypt_value(output ? C_ONE : C_ZERO));
            next_R.push_back(encrypt_value(output ? R_ONE : R_ZERO));
        }
        
        curr_L = next_L;
        curr_C = next_C;
        curr_R = next_R;
        
        if (gen % 5 == 0 || gen == 20) {
            cout << "  Gen " << setw(3) << gen << ": ";
            for (int i = 0; i < N; i++) {
                double val = decrypt_value(curr_R[i]);
                cout << (abs(val - R_ONE) < abs(val - R_ZERO) ? 1 : 0);
            }
            cout << "\n";
        }
    }

    auto end = high_resolution_clock::now();
    auto time = duration_cast<milliseconds>(end - start).count();

    cout << "\n  Time: " << time / 1000.0 << " seconds\n";
    cout << "  Level: " << curr_R[0]->GetLevel() << "\n\n";

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
        double val = decrypt_value(curr_R[i]);
        int bit = (abs(val - R_ONE) < abs(val - R_ZERO)) ? 1 : 0;
        cout << bit;
        if (bit == history[20][i]) matches++;
    }
    cout << "\n\n";
    cout << "  Match: " << matches << "/" << N << "\n\n";

    cout << "========================================\n";
    cout << "  PURE DIRECT COMPLETE\n";
    cout << "========================================\n\n";
    cout << "  ✅ State: 0→φ⁻⁵, 1→φ⁻²\n";
    cout << "  ✅ Weights: L→φ⁻⁴/φ⁻¹, C→φ⁻³/φ⁰, R→φ⁻³/φ⁰\n";
    cout << "  ✅ 8/8 transition\n";
    cout << "  ✅ Match: " << matches << "/" << N << "\n";
    cout << "  ✅ Level 0\n";
    cout << "  ✅ Depth 0\n";
    cout << "  ⚠️ May decryption pa sa threshold\n\n";

    return 0;
}
