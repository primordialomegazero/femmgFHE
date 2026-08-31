// ============================================
// φ-RULE 110 TRIPLE — TATLONG VERSIONS
//
// Ang bawat cell ay may tatlong encrypted values:
// L-value, C-value, R-value
//
// Sa transition, pure EvalAdd lang:
// sum = L_value[i-1] + C_value[i] + R_value[i+1]
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
    cout << "  φ-RULE 110 TRIPLE\n";
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
    
    // Positional φ-values
    const double L_ZERO = pow(PHI, -4);  // 0.146
    const double L_ONE = pow(PHI, -1);   // 0.618
    const double C_ZERO = pow(PHI, -3);  // 0.236
    const double C_ONE = pow(PHI, 0);    // 1.0
    const double R_ZERO = pow(PHI, -3);  // 0.236
    const double R_ONE = pow(PHI, 0);    // 1.0

    cout << "  ✅ CKKS initialized (depth 0!)\n";
    cout << "  L: 0→" << L_ZERO << ", 1→" << L_ONE << "\n";
    cout << "  C: 0→" << C_ZERO << ", 1→" << C_ONE << "\n";
    cout << "  R: 0→" << R_ZERO << ", 1→" << R_ONE << "\n\n";

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
    // TRIPLE STATE ENCODING
    // ============================================
    //
    // Ang bawat cell ay may tatlong versions:
    // - L_version: φ-value para sa L position
    // - C_version: φ-value para sa C position
    // - R_version: φ-value para sa R position
    //
    // Sa transition:
    // sum = L_version[i-1] + C_version[i] + R_version[i+1]
    //
    // PURE EVALADD! Walang multiplication, walang conversion!

    cout << "========================================\n";
    cout << "  TRIPLE STATE EVOLUTION\n";
    cout << "========================================\n\n";

    // Initial state: I-encrypt bilang triple versions
    vector<Ciphertext<DCRTPoly>> L_cells, C_cells, R_cells;
    
    for (int bit : history[0]) {
        L_cells.push_back(encrypt_value(bit ? L_ONE : L_ZERO));
        C_cells.push_back(encrypt_value(bit ? C_ONE : C_ZERO));
        R_cells.push_back(encrypt_value(bit ? R_ONE : R_ZERO));
    }

    cout << "  Gen 0: ";
    for (int i = 0; i < N; i++) cout << history[0][i];
    cout << "\n\n";

    auto start = high_resolution_clock::now();
    
    vector<Ciphertext<DCRTPoly>> curr_L = L_cells;
    vector<Ciphertext<DCRTPoly>> curr_C = C_cells;
    vector<Ciphertext<DCRTPoly>> curr_R = R_cells;

    for (int gen = 1; gen <= 20; gen++) {
        vector<Ciphertext<DCRTPoly>> next_L, next_C, next_R;
        
        for (int i = 0; i < N; i++) {
            // PURE EVALADD:
            // sum = L_version[i-1] + C_version[i] + R_version[i+1]
            auto sum1 = cc->EvalAdd(curr_L[(i + N - 1) % N], curr_C[i]);
            auto sum2 = cc->EvalAdd(sum1, curr_R[(i + 1) % N]);
            
            // ANG PROBLEMA: Kailangan nating i-convert ang sum
            // pabalik sa triple versions para sa susunod na generation.
            //
            // Ang sum ay nasa range [0.618, 2.618].
            // Ang output ay 0 o 1 (mod 2 ng floor).
            //
            // Sa ngayon, i-decrypt muna para sa testing.
            double sum_val = decrypt_value(sum2);
            int output = ((int)floor(sum_val)) % 2;
            
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
    cout << "  TRIPLE COMPLETE\n";
    cout << "========================================\n\n";
    cout << "  ✅ Triple state encoding\n";
    cout << "  ✅ Pure EvalAdd transition\n";
    cout << "  ✅ Match: " << matches << "/" << N << "\n";
    cout << "  ✅ Level 0\n";
    cout << "  ✅ Depth 0\n";
    cout << "  ⚠️ May decryption pa sa normalization\n\n";

    return 0;
}
