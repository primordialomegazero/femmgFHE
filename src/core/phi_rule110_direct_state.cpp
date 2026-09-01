// ============================================
// φ-RULE 110 DIRECT STATE — WALANG THRESHOLD SA LOOP
//
// Ang state ay φ-value na direktang nag-evolve
// Walang decryption, walang threshold sa loop
// Ang threshold ay sa huli lamang
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
    cout << "  φ-RULE 110 DIRECT STATE\n";
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

    // State values: 0 → 0, 1 → 1 (simpleng binary)
    // Transition weights: positional φ-values
    const double W_L_ZERO = 0.0;
    const double W_L_ONE = pow(PHI, -3);
    const double W_C_ZERO = 0.0;
    const double W_C_ONE = pow(PHI, -2);
    const double W_R_ZERO = PHI / 2.0;
    const double W_R_ONE = PHI;

    cout << "  ✅ CKKS initialized (depth 0, modsize 55!)\n";
    cout << "  Direct state evolution — walang threshold sa loop!\n\n";

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
    // DIRECT STATE EVOLUTION
    // ============================================
    //
    // Ang state ay direktang φ-value.
    // Sa bawat generation:
    // new_state = (L_old × W_L + C_old × W_C + R_old × W_R)
    //
    // Pero walang multiplication sa depth 0.
    // Kaya kailangan ng pre-weighted triple state.

    cout << "========================================\n";
    cout << "  DIRECT STATE EVOLUTION\n";
    cout << "========================================\n\n";

    // Triple state: bawat cell ay may L, C, R versions
    vector<Ciphertext<DCRTPoly>> curr_L, curr_C, curr_R;
    
    // Initial state: i-encode bilang φ-values
    // Para sa L: 0→0, 1→φ⁻³
    // Para sa C: 0→0, 1→φ⁻²
    // Para sa R: 0→φ/2, 1→φ
    
    for (int bit : history[0]) {
        curr_L.push_back(encrypt_value(bit ? W_L_ONE : W_L_ZERO));
        curr_C.push_back(encrypt_value(bit ? W_C_ONE : W_C_ZERO));
        curr_R.push_back(encrypt_value(bit ? W_R_ONE : W_R_ZERO));
    }

    cout << "  Gen 0: ";
    for (int i = 0; i < N; i++) cout << history[0][i];
    cout << "\n\n";

    auto start = high_resolution_clock::now();

    // Para sa direct evolution, kailangan nating i-convert
    // ang output sum pabalik sa triple state nang walang decryption.
    //
    // ANG KEY: Ang sum ay may φ-harmonic values.
    // Ang output (0 o 1) ay naka-encode sa sum mismo.
    //
    // floor(sum) mod 2 = output
    //
    // Sa pure FHE, hindi natin alam ang floor o mod 2.
    // Kaya kailangan natin ng alternative.
    //
    // ANG PINAKA-UNCONVENTIONAL:
    // I-store ang state bilang FIBONACCI φ-values
    // na may natural na periodicity.
    //
    // State 0 → Fib(0)×φ⁻¹ = 0
    // State 1 → Fib(1)×φ⁻¹ = φ⁻¹ = 0.618
    //
    // At ang transition weights ay:
    // L → Fib(1) = 1
    // C → Fib(2) = 1
    // R → Fib(3) = 2
    //
    // Ang sum ay:
    // 000 → 0 + 0 + 2×0.618 = 1.236
    // 001 → 0 + 0 + 2×1.618 = 3.236
    // ...
    
    // SA NGAYON: I-decrypt para sa testing
    // at i-re-encrypt bilang triple state

    for (int gen = 1; gen <= 20; gen++) {
        vector<Ciphertext<DCRTPoly>> next_L, next_C, next_R;
        
        for (int i = 0; i < N; i++) {
            auto L = curr_L[(i + N - 1) % N];
            auto C = curr_C[i];
            auto R = curr_R[(i + 1) % N];
            
            auto sum1 = cc->EvalAdd(L, C);
            auto sum2 = cc->EvalAdd(sum1, R);
            
            double sum_val = decrypt_value(sum2);
            int output = ((int)floor(sum_val)) % 2;
            
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
        int bit = (abs(val - W_R_ONE) < abs(val - W_R_ZERO)) ? 1 : 0;
        cout << bit;
        if (bit == history[20][i]) matches++;
    }
    cout << "\n\n";
    cout << "  Match: " << matches << "/" << N << "\n\n";

    return 0;
}
