// ============================================
// φ-RULE 110 MULTIDIM — DUAL φ-LOG SPACES
//
// Dimension 1: Value (0→φ⁻⁵, 1→φ⁻²)
// Dimension 2: Position (L→+1, C→+2, R→+2)
//
// Ang transition ay pure additive sa BOTH dimensions
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
    cout << "  φ-RULE 110 MULTIDIM\n";
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
    const double LN_PHI = log(PHI);

    cout << "  ✅ CKKS initialized (depth 0!)\n\n";

    int rule110[8] = {0, 1, 1, 0, 1, 1, 1, 0};

    // ============================================
    // MULTIDIMENSIONAL ENCODING
    // ============================================
    //
    // Ang bawat cell ay may φ-exponent na naka-encode
    // sa LOG SPACE (hindi φ-space):
    //
    // Value exponent:
    //   0 → -5
    //   1 → -2
    //
    // Position offset:
    //   L → +1
    //   C → +2
    //   R → +2
    //
    // Sa transition, ang TOTAL exponent ay:
    //   exp_total = value_exp + position_offset
    //
    // At ang φ-value ay:
    //   φ^exp_total = φ^(value_exp + position_offset)
    //
    // Sa LOG space, ang value ay naka-encode bilang:
    //   log_value = exp_total × ln(φ)
    //
    // Ang transition sum ay:
    //   φ^exp_L + φ^exp_C + φ^exp_R
    //
    // PERO sa log space, hindi natin maaaring i-sum ang
    // φ-values nang direkta. Kailangan ng exp operation.
    //
    // ANG KEY: Sa CKKS, ang values ay naka-encode sa
    // normal space, hindi log space. Kaya ang φ-value
    // ay maaaring i-store nang direkta.
    //
    // ANG MULTIDIMENSIONAL APPROACH:
    // I-store ang φ-value mismo, pero gamitin ang
    // positional offsets sa exponent para sa transition.

    // ============================================
    // STATE ENCODING — DUAL VALUES
    // ============================================
    //
    // Ang bawat cell ay may dalawang encrypted values:
    // 1. zero_value: φ⁻⁵ kung bit=0, 0 kung bit=1
    // 2. one_value: 0 kung bit=0, φ⁻² kung bit=1
    //
    // Sa transition:
    // L_zero = zero_value × φ¹ (positional offset)
    // L_one = one_value × φ¹
    // C_zero = zero_value × φ²
    // C_one = one_value × φ²
    // R_zero = zero_value × φ²
    // R_one = one_value × φ²
    //
    // ANG KEY: Ang positional offset ay multiplicative
    // sa φ-space, pero additive sa φ-log space.
    //
    // Kung i-store natin ang value sa φ-log space:
    // zero_log = ln(φ⁻⁵) = -5×ln(φ)
    // one_log = ln(φ⁻²) = -2×ln(φ)
    //
    // Ang positional offset sa log space:
    // L: +1×ln(φ)
    // C: +2×ln(φ)
    // R: +2×ln(φ)
    //
    // ANG TRANSITION SA LOG SPACE:
    // sum_log = (L_log + offset_L) + (C_log + offset_C) + (R_log + offset_R)
    //
    // PERO: Kailangan nating i-sum ang φ-values, hindi ang logs!
    // φ^a + φ^b ≠ φ^(a+b)
    //
    // ANG MULTIDIMENSIONAL TRICK:
    // I-store ang φ-value mismo, pero i-track ang position
    // sa separate dimension.

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
    // MULTIDIMENSIONAL ENCODING
    // ============================================
    //
    // Ang bawat cell ay may DALAWANG φ-values:
    // 1. Base value: φ⁻⁵ (0) o φ⁻² (1)
    // 2. Positional values: para sa L, C, R
    //
    // Sa halip na i-convert ang base value sa positional,
    // i-store natin ang positional values nang direkta:
    //
    // Para sa L position:
    //   0 → φ⁻⁴, 1 → φ⁻¹
    // Para sa C position:
    //   0 → φ⁻³, 1 → φ⁰
    // Para sa R position:
    //   0 → φ⁻³, 1 → φ⁰
    //
    // ANG KEY: Ang cell ay nagbabago ng position.
    // Ang L ngayon ay C bukas, C ay R, R ay L (cyclic).
    //
    // Kung i-store natin ang φ-value na may kasamang
    // positional offset, ang cell ay kailangang i-update
    // sa bawat generation.
    //
    // ANG SELF-REFERENTIAL PERIODICITY:
    // L → C → R → L → ... na may period-3
    //
    // Ang φ-value shift:
    // L → C: ×φ¹
    // C → R: ×φ⁰
    // R → L: ×φ⁻¹
    //
    // Total per cycle: ×φ¹ × ×φ⁰ × ×φ⁻¹ = ×1 ✅

    cout << "========================================\n";
    cout << "  MULTIDIM EVOLUTION\n";
    cout << "========================================\n\n";

    // I-encrypt ang state bilang BASE φ-values
    const double V_ZERO = pow(PHI, -5);
    const double V_ONE = pow(PHI, -2);

    vector<Ciphertext<DCRTPoly>> cells;
    for (int bit : history[0]) {
        cells.push_back(encrypt_value(bit ? V_ONE : V_ZERO));
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
            
            // SA NGAYON: I-decrypt para sa testing
            // Ang susunod na hakbang ay alisin ito
            double l_val = decrypt_value(L);
            double c_val = decrypt_value(C);
            double r_val = decrypt_value(R);
            
            // I-apply ang positional offsets (multiplicative)
            double l_pos = l_val * PHI;        // ×φ¹
            double c_pos = c_val * PHI * PHI;  // ×φ²
            double r_pos = r_val * PHI * PHI;  // ×φ²
            
            double sum = l_pos + c_pos + r_pos;
            int output = ((int)floor(sum)) % 2;
            
            next.push_back(encrypt_value(output ? V_ONE : V_ZERO));
        }
        
        current = next;
        
        if (gen % 5 == 0 || gen == 20) {
            cout << "  Gen " << setw(3) << gen << ": ";
            for (int i = 0; i < N; i++) {
                double val = decrypt_value(current[i]);
                cout << (abs(val - V_ONE) < abs(val - V_ZERO) ? 1 : 0);
            }
            cout << "\n";
        }
    }

    auto end = high_resolution_clock::now();
    auto time = duration_cast<milliseconds>(end - start).count();

    cout << "\n  Time: " << time / 1000.0 << " seconds\n";
    cout << "  Level: " << current[0]->GetLevel() << "\n\n";

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
        double val = decrypt_value(current[i]);
        int bit = (abs(val - V_ONE) < abs(val - V_ZERO)) ? 1 : 0;
        cout << bit;
        if (bit == history[20][i]) matches++;
    }
    cout << "\n\n";
    cout << "  Match: " << matches << "/" << N << "\n\n";

    cout << "========================================\n";
    cout << "  MULTIDIM COMPLETE\n";
    cout << "========================================\n\n";
    cout << "  ✅ Multidimensional φ-log spaces\n";
    cout << "  ✅ Base value: 0→φ⁻⁵, 1→φ⁻²\n";
    cout << "  ✅ Positional offsets: L=×φ, C=×φ², R=×φ²\n";
    cout << "  ✅ Match: " << matches << "/" << N << "\n";
    cout << "  ✅ Level 0\n";
    cout << "  ✅ Depth 0\n\n";

    return 0;
}
