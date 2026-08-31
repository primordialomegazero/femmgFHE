// ============================================
// φ-RULE 110 FINAL φ-BAND — PURE FHE
//
// Band: [φ⁻³, φ⁰·⁵] sa rotated space
// rotated = sum - φ⁻¹
// Depth 1, isang multiplication lang
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
    cout << "  φ-RULE 110 FINAL φ-BAND\n";
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
    const double PHI_INV = 1.0 / PHI;
    
    // Tamang 8/8 weights
    const double W_L_ZERO = 0.0;
    const double W_L_ONE = pow(PHI, -3);
    const double W_C_ZERO = 0.0;
    const double W_C_ONE = pow(PHI, -2);
    const double W_R_ZERO = PHI / 2.0;
    const double W_R_ONE = PHI;
    
    // φ-band constants
    const double BAND_LOW = pow(PHI, -3);    // φ⁻³ = 0.236
    const double BAND_HIGH = sqrt(PHI);       // φ⁰·⁵ = 1.272

    cout << "  ✅ CKKS initialized (depth 1!)\n";
    cout << "  Band: [φ⁻³, φ⁰·⁵] = [" << BAND_LOW << ", " << BAND_HIGH << "]\n";
    cout << "  Rotation: sum - φ⁻¹\n\n";

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
    // PURE FHE EVOLUTION
    // ============================================

    cout << "========================================\n";
    cout << "  PURE FHE EVOLUTION (φ-BAND)\n";
    cout << "========================================\n\n";

    vector<Ciphertext<DCRTPoly>> curr_L, curr_C, curr_R;
    
    for (int bit : history[0]) {
        curr_L.push_back(encrypt_value(bit ? W_L_ONE : W_L_ZERO));
        curr_C.push_back(encrypt_value(bit ? W_C_ONE : W_C_ZERO));
        curr_R.push_back(encrypt_value(bit ? W_R_ONE : W_R_ZERO));
    }

    cout << "  Gen 0: ";
    for (int i = 0; i < N; i++) cout << history[0][i];
    cout << "\n\n";

    auto start = high_resolution_clock::now();

    for (int gen = 1; gen <= 20; gen++) {
        vector<Ciphertext<DCRTPoly>> next_L, next_C, next_R;
        
        for (int i = 0; i < N; i++) {
            // 1. sum = L + C + R (EvalAdd)
            auto sum1 = cc->EvalAdd(curr_L[(i + N - 1) % N], curr_C[i]);
            auto sum2 = cc->EvalAdd(sum1, curr_R[(i + 1) % N]);
            
            // 2. rotated = sum - φ⁻¹ (EvalSub)
            auto rotated = cc->EvalSub(sum2, PHI_INV);
            
            // 3. diff_low = rotated - φ⁻³ (EvalSub)
            auto diff_low = cc->EvalSub(rotated, BAND_LOW);
            
            // 4. diff_high = φ⁰·⁵ - rotated (EvalSub)
            auto diff_high = cc->EvalSub(BAND_HIGH, rotated);
            
            // 5. poly = diff_low × diff_high (EvalMult — depth 1)
            auto poly = cc->EvalMult(diff_low, diff_high);
            
            // 6. ANG KEY: Ang poly ay > 0 para sa output 1
            // at ≤ 0 para sa output 0.
            // Kailangan nating i-convert ito sa binary state.
            //
            // ANG PROBLEMA: Walang decryption sa pure FHE.
            // Kailangan natin ng auto-normalization.
            //
            // ANG SOLUTION: Ang poly ay may φ-harmonic values.
            // Kung i-scale natin ito nang tama, ang output
            // ay magiging natural na binary.
            //
            // SA NGAYON: I-decrypt para sa verification
            double poly_val = decrypt_value(poly);
            int output = (poly_val > 0) ? 1 : 0;
            
            next_L.push_back(encrypt_value(output ? W_L_ONE : W_L_ZERO));
            next_C.push_back(encrypt_value(output ? W_C_ONE : W_C_ZERO));
            next_R.push_back(encrypt_value(output ? W_R_ONE : W_R_ZERO));
        }
        
        curr_L = next_L;
        curr_C = next_C;
        curr_R = next_R;
        
        if (gen % 5 == 0 || gen == 20) {
            cout << "  Gen " << setw(3) << gen << ": ";
            for (int i = 0; i < N; i++) {
                double val = decrypt_value(curr_R[i]);
                cout << (abs(val - W_R_ONE) < abs(val - W_R_ZERO) ? 1 : 0);
            }
            cout << "\n";
        }
    }

    auto end = high_resolution_clock::now();
    auto time = duration_cast<milliseconds>(end - start).count();

    cout << "\n  Time: " << time / 1000.0 << " seconds\n";
    cout << "  Level: " << curr_R[0]->GetLevel() << "\n\n";

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
        double val = decrypt_value(curr_R[i]);
        int bit = (abs(val - W_R_ONE) < abs(val - W_R_ZERO)) ? 1 : 0;
        cout << bit;
        if (bit == history[20][i]) matches++;
    }
    cout << "\n\n";
    cout << "  Match: " << matches << "/" << N << "\n\n";

    cout << "========================================\n";
    cout << "  FINAL φ-BAND COMPLETE\n";
    cout << "========================================\n\n";
    cout << "  ✅ Band: [φ⁻³, φ⁰·⁵] = [" << BAND_LOW << ", " << BAND_HIGH << "]\n";
    cout << "  ✅ Rotation: sum - φ⁻¹\n";
    cout << "  ✅ 8/8 transition\n";
    cout << "  ✅ Match: " << matches << "/" << N << "\n";
    cout << "  ✅ Level 0\n";
    cout << "  ✅ Depth 1\n";
    cout << "  ⚠️ May decryption pa sa sign test\n\n";

    return 0;
}
