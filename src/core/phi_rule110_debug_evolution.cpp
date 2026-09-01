// ============================================
// φ-RULE 110 DEBUG EVOLUTION — PRINT LAHAT
//
// I-print ang sums, floors, at outputs sa bawat gen
// Para makita kung saan nagkakamali
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
    cout << "  φ-RULE 110 DEBUG EVOLUTION\n";
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
    cc->EvalMultKeyGen(keyPair.secretKey);

    const double PHI = 1.6180339887498948482;

    const double W_L_ZERO = 0.0;
    const double W_L_ONE = pow(PHI, -3);
    const double W_C_ZERO = 0.0;
    const double W_C_ONE = pow(PHI, -2);
    const double W_R_ZERO = PHI / 2.0;
    const double W_R_ONE = PHI;

    int rule110[8] = {0, 1, 1, 0, 1, 1, 1, 0};

    auto encrypt_weighted = [&](int bit, double weight) {
        double val = bit ? weight : 0.0;
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
    // DEBUG EVOLUTION
    // ============================================

    cout << "========================================\n";
    cout << "  DEBUG EVOLUTION\n";
    cout << "========================================\n\n";

    vector<Ciphertext<DCRTPoly>> curr_L, curr_C, curr_R;
    
    for (int bit : history[0]) {
        curr_L.push_back(encrypt_weighted(bit, W_L_ONE));
        curr_C.push_back(encrypt_weighted(bit, W_C_ONE));
        curr_R.push_back(encrypt_weighted(bit, W_R_ONE));
    }

    cout << "  Gen 0 (initial):\n";
    cout << "  Plaintext: ";
    for (int i = 0; i < N; i++) cout << history[0][i];
    cout << "\n";
    cout << "  L values: ";
    for (int i = 0; i < N; i++) cout << fixed << setprecision(3) << decrypt_value(curr_L[i]) << " ";
    cout << "\n";
    cout << "  C values: ";
    for (int i = 0; i < N; i++) cout << fixed << setprecision(3) << decrypt_value(curr_C[i]) << " ";
    cout << "\n";
    cout << "  R values: ";
    for (int i = 0; i < N; i++) cout << fixed << setprecision(3) << decrypt_value(curr_R[i]) << " ";
    cout << "\n\n";

    for (int gen = 1; gen <= 5; gen++) {
        vector<Ciphertext<DCRTPoly>> next_L, next_C, next_R;
        vector<double> sums(N);
        vector<int> outputs(N);
        
        for (int i = 0; i < N; i++) {
            auto L = curr_L[(i + N - 1) % N];
            auto C = curr_C[i];
            auto R = curr_R[(i + 1) % N];
            
            auto sum1 = cc->EvalAdd(L, C);
            auto sum2 = cc->EvalAdd(sum1, R);
            
            double sum_val = decrypt_value(sum2);
            sums[i] = sum_val;
            int output = ((int)floor(sum_val)) % 2;
            outputs[i] = output;
            
            next_L.push_back(encrypt_weighted(output, W_L_ONE));
            next_C.push_back(encrypt_weighted(output, W_C_ONE));
            next_R.push_back(encrypt_weighted(output, W_R_ONE));
        }
        
        curr_L = next_L;
        curr_C = next_C;
        curr_R = next_R;
        
        cout << "  Gen " << setw(1) << gen << " (debug):\n";
        cout << "  Expected: ";
        for (int i = 0; i < N; i++) cout << history[gen][i];
        cout << "\n";
        cout << "  Sums:    ";
        for (int i = 0; i < N; i++) cout << setw(7) << fixed << setprecision(3) << sums[i];
        cout << "\n";
        cout << "  Floors:  ";
        for (int i = 0; i < N; i++) cout << setw(7) << (int)floor(sums[i]);
        cout << "\n";
        cout << "  Mod2:    ";
        for (int i = 0; i < N; i++) cout << setw(7) << ((int)floor(sums[i])) % 2;
        cout << "\n";
        cout << "  Outputs: ";
        for (int i = 0; i < N; i++) cout << setw(7) << outputs[i];
        cout << "\n\n";
    }

    // ============================================
    // TRANSITION TABLE ANALYSIS
    // ============================================

    cout << "========================================\n";
    cout << "  TRANSITION TABLE ANALYSIS\n";
    cout << "========================================\n\n";

    cout << "  L C R | L_val | C_val | R_val | Sum      | Floor | Expected\n";
    cout << "  ------|-------|-------|-------|----------|-------|----------\n";

    for (int L : {0, 1}) {
        for (int C : {0, 1}) {
            for (int R : {0, 1}) {
                double l_val = L ? W_L_ONE : W_L_ZERO;
                double c_val = C ? W_C_ONE : W_C_ZERO;
                double r_val = R ? W_R_ONE : W_R_ZERO;
                double sum = l_val + c_val + r_val;
                int floor_val = (int)floor(sum);
                int expected = rule110[(L << 2) | (C << 1) | R];
                
                cout << "  " << L << " " << C << " " << R << " | "
                     << setw(5) << fixed << setprecision(3) << l_val << " | "
                     << setw(5) << c_val << " | "
                     << setw(5) << r_val << " | "
                     << setw(8) << sum << " | "
                     << setw(5) << floor_val << " | "
                     << setw(8) << expected << "\n";
            }
        }
    }

    return 0;
}
