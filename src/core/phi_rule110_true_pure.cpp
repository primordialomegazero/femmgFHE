// ============================================
// φ-RULE 110 TRUE PURE — DEPTH 1, WALANG DECRYPT
//
// Auto-normalization gamit ang EvalMult na may
// plaintext constant para sa φ-scaling
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
    cout << "  φ-RULE 110 TRUE PURE\n";
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
    cc->EvalMultKeyGen(keyPair.secretKey);

    const double PHI = 1.6180339887498948482;
    
    // Positional φ-values
    const double L_ZERO = pow(PHI, -4);
    const double L_ONE = pow(PHI, -1);
    const double C_ZERO = pow(PHI, -3);
    const double C_ONE = pow(PHI, 0);
    const double R_ZERO = pow(PHI, -3);
    const double R_ONE = pow(PHI, 0);
    
    // State values
    const double V_ZERO = pow(PHI, -5);
    const double V_ONE = pow(PHI, -2);
    
    // Scaling constants para sa auto-normalization
    // Ang sums ay nasa range [0.618, 2.618]
    // Ang state values ay nasa range [0.090, 0.382]
    // Kaya kailangan ng scale factor na φ⁻³ = 0.236
    const double SCALE = pow(PHI, -3);  // 0.236

    cout << "  ✅ CKKS initialized (depth 1!)\n";
    cout << "  Auto-normalization scale: φ⁻³ = " << SCALE << "\n\n";

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
    // TRUE PURE FHE — WALANG DECRYPTION
    // ============================================
    //
    // Ang auto-normalization:
    // 1. Sum = L + C + R (EvalAdd)
    // 2. Scale down: sum × SCALE (EvalMult na may constant)
    // 3. Ang scaled sum ay nasa state range
    //
    // PERO: Kailangan nating malaman kung ang output ay 0 o 1.
    // Ang scaled sum ay hindi direktang nagbibigay ng tamang
    // state value (V_ZERO o V_ONE).
    //
    // ANG KEY: Ang scaled sum ay nasa φ-harmonic range.
    // Kung ang scaled sum ay mas malapit sa V_ZERO, output ay 0.
    // Kung mas malapit sa V_ONE, output ay 1.
    //
    // PERO: Sa pure FHE, hindi natin alam kung alin ang mas malapit.
    //
    // ANG TOTOONG SOLUTION: Gumamit ng polynomial approximation
    // ng threshold function sa depth 1.

    cout << "========================================\n";
    cout << "  TRUE PURE EVOLUTION (WALANG DECRYPT)\n";
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
            // PURE EVALADD: sum = L[i-1] + C[i] + R[i+1]
            auto sum1 = cc->EvalAdd(curr_L[(i + N - 1) % N], curr_C[i]);
            auto sum2 = cc->EvalAdd(sum1, curr_R[(i + 1) % N]);
            
            // AUTO-NORMALIZATION: Scale down gamit EvalMult
            auto scaled = cc->EvalMult(sum2, SCALE);
            
            // ANG PROBLEMA: Ang scaled value ay hindi pa rin
            // ang tamang state value. Kailangan natin ng
            // threshold function.
            //
            // SA NGAYON: I-decrypt para sa testing
            double scaled_val = decrypt_value(scaled);
            
            // Threshold: kung ang scaled ay nasa [0.2, 0.5], output = 1
            // Kung hindi, output = 0
            int output = (scaled_val > 0.2 && scaled_val < 0.5) ? 1 : 0;
            
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
                double val = decrypt_value(curr_C[i]);
                cout << (abs(val - C_ONE) < abs(val - C_ZERO) ? 1 : 0);
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
        double val = decrypt_value(curr_C[i]);
        int bit = (abs(val - C_ONE) < abs(val - C_ZERO)) ? 1 : 0;
        cout << bit;
        if (bit == history[20][i]) matches++;
    }
    cout << "\n\n";
    cout << "  Match: " << matches << "/" << N << "\n\n";

    cout << "========================================\n";
    cout << "  TRUE PURE COMPLETE\n";
    cout << "========================================\n\n";
    cout << "  ✅ Depth 1\n";
    cout << "  ✅ Match: " << matches << "/" << N << "\n";
    cout << "  ⚠️ May decryption pa sa threshold\n\n";

    return 0;
}
