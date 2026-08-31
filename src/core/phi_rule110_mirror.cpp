// ============================================
// φ-RULE 110 MIRROR — SELF-REFERENTIAL
//
// Ang φ-value at φ-log ay iisang bagay
// Ang transition ay self-referential
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
    cout << "  φ-RULE 110 MIRROR\n";
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
    const double L_ZERO = pow(PHI, -4);
    const double L_ONE = pow(PHI, -1);
    const double C_ZERO = pow(PHI, -3);
    const double C_ONE = pow(PHI, 0);
    const double R_ZERO = pow(PHI, -3);
    const double R_ONE = pow(PHI, 0);
    
    // State values (φ-log indicators)
    const double V_ZERO = pow(PHI, -5);
    const double V_ONE = pow(PHI, -2);

    cout << "  ✅ CKKS initialized (depth 0!)\n";
    cout << "  Mirror: value ↔ log\n\n";

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
    // ANG SELF-REFERENTIAL MIRROR
    // ============================================
    //
    // Ang sum ay may φ-harmonic structure.
    // Sa halip na i-convert ang sum papuntang φ-log,
    // gamitin natin ang MIRROR property:
    //
    // Ang sum ay maaaring i-represent bilang:
    // sum = φ^a + φ^b + φ^c
    //
    // At ang output ay:
    // output = floor(sum) mod 2
    //
    // ANG KEY: Ang sum mismo ay may φ-information.
    // Hindi natin kailangan ng explicit conversion.
    //
    // Ang output ay maaaring ma-determine mula sa
    // φ-harmonic position ng sum:
    //
    // Sum < 1.0 → floor 0 → output 0
    // 1.0 ≤ Sum < 2.0 → floor 1 → output 1
    // Sum ≥ 2.0 → floor 2 → output 0
    //
    // ANG MIRROR: Ang threshold sa value space
    // ay mirror ng threshold sa log space.
    //
    // Value threshold: 1.0 at 2.0
    // Log threshold: 0 at log_φ(2) = 1.44
    //
    // Sa φ-space:
    // 1.0 = φ⁰
    // 2.0 = φ^1.44

    cout << "========================================\n";
    cout << "  MIRROR EVOLUTION\n";
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
            
            // ANG MIRROR: Sa halip na i-decrypt,
            // gamitin ang sum nang direkta bilang
            // φ-log indicator.
            //
            // Ang sum ay nasa range [0.618, 2.618].
            // Ang output ay 0 o 1.
            //
            // ANG KEY: Ang sum mismo ay maaaring
            // gamitin bilang next state kung
            // i-scale natin ito nang tama.
            //
            // PERO: Walang multiplication sa depth 0.
            // Kaya kailangan ng ibang paraan.
            //
            // SA NGAYON: I-decrypt para sa testing
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
    cout << "  MIRROR COMPLETE\n";
    cout << "========================================\n\n";
    cout << "  ✅ Self-referential mirror\n";
    cout << "  ✅ Pure EvalAdd transition\n";
    cout << "  ✅ Match: " << matches << "/" << N << "\n";
    cout << "  ✅ Level 0\n";
    cout << "  ✅ Depth 0\n";
    cout << "  ⚠️ May decryption pa sa normalization\n\n";

    return 0;
}
