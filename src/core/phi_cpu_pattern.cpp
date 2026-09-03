// ============================================
// φ-CPU PATTERN — HANAPIN ANG TAMANG DECODE
//
// I-print lahat ng values para sa lahat ng
// 8 combinations — hanapin ang pattern!
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
    cout << "  φ-CPU PATTERN ANALYSIS\n";
    cout << "========================================\n\n";

    CCParams<CryptoContextCKKSRNS> parameters;
    parameters.SetMultiplicativeDepth(1);
    parameters.SetScalingModSize(50);
    parameters.SetBatchSize(64);
    parameters.SetSecurityLevel(HEStd_128_classic);

    CryptoContext<DCRTPoly> cc = GenCryptoContext(parameters);
    cc->Enable(PKE);
    cc->Enable(KEYSWITCH);
    cc->Enable(LEVELEDSHE);

    auto keyPair = cc->KeyGen();

    const double PHI = 1.6180339887498948482;
    const double PHI_INV = 1.0 / PHI;

    auto encrypt_emergent = [&](int bit, int alu_id) {
        vector<double> v(64, 0.0);
        double val = (bit == 0) ? PHI : PHI_INV;
        int base = alu_id * 4;
        for (int i = 0; i < 4; i++) v[base + i] = val;
        Plaintext pt = cc->MakeCKKSPackedPlaintext(v);
        return cc->Encrypt(keyPair.publicKey, pt);
    };

    auto decrypt_emergent = [&](const Ciphertext<DCRTPoly>& ct) {
        Plaintext result_pt;
        cc->Decrypt(keyPair.secretKey, ct, &result_pt);
        result_pt->SetLength(64);
        vector<double> results(64);
        for (int i = 0; i < 64; i++) {
            results[i] = result_pt->GetCKKSPackedValue()[i].real();
        }
        return results;
    };

    cout << "  LAHAT NG 8 COMBINATIONS:\n\n";
    cout << "  A B Cin | Sum Avg | Normalized | Mod2 | Expected Sum | Match?\n";
    cout << "  --------|---------|------------|------|-------------|--------\n";

    for (int A : {0, 1}) {
        for (int B : {0, 1}) {
            for (int Cin : {0, 1}) {
                auto ct_a = encrypt_emergent(A, 0);
                auto ct_b = encrypt_emergent(B, 1);
                auto ct_cin = encrypt_emergent(Cin, 2);
                
                auto ct_stage1 = cc->EvalAdd(ct_a, ct_b);
                auto ct_sum = cc->EvalAdd(ct_stage1, ct_cin);
                
                auto sum_vals = decrypt_emergent(ct_sum);
                double sum_avg = (sum_vals[8] + sum_vals[9] + sum_vals[10] + sum_vals[11]) / 4.0;
                
                double normalized = (PHI - sum_avg) / (PHI - PHI_INV);
                double mod2 = normalized - 2.0 * floor(normalized / 2.0);
                if (mod2 > 1.0) mod2 = 2.0 - mod2;
                
                int exp_sum = (A + B + Cin) % 2;
                bool match = (round(mod2) == exp_sum);
                
                cout << "  " << A << " " << B << " " << Cin << " | "
                     << setw(7) << fixed << setprecision(3) << sum_avg << " | "
                     << setw(10) << normalized << " | "
                     << setw(4) << mod2 << " | "
                     << setw(11) << exp_sum << " | "
                     << (match ? "✅" : "❌") << "\n";
            }
        }
    }

    cout << "\n";

    // ============================================
    // PATTERN ANALYSIS
    // ============================================

    cout << "========================================\n";
    cout << "  PATTERN ANALYSIS\n";
    cout << "========================================\n\n";

    cout << "  Sum Avg values:\n";
    cout << "  000 → 1.618 (φ) → normalized 0 → sum 0\n";
    cout << "  001 → 0.618 (φ⁻¹) → normalized 1 → sum 1\n";
    cout << "  010 → 1.618 (φ) → normalized 0 → sum 0 ← MALI!\n";
    cout << "  011 → 0.618 (φ⁻¹) → normalized 1 → sum 1\n";
    cout << "  100 → 1.618 (φ) → normalized 0 → sum 0 ← MALI!\n";
    cout << "  101 → 0.618 (φ⁻¹) → normalized 1 → sum 1\n";
    cout << "  110 → 2.236 (?) → normalized ? → sum ?\n";
    cout << "  111 → ? → ? → sum ?\n\n";

    cout << "  ANG PROBLEMA:\n";
    cout << "  010 at 100 ay may SUM = 1 — pero ang decode\n";
    cout << "  ay nagbibigay ng 0 kasi ang avg ay φ (hindi φ⁻¹)\n\n";

    cout << "  ANG TAMANG PATTERN:\n";
    cout << "  Sum = 0 kapag (A+B+Cin) ay even (0, 2)\n";
    cout << "  Sum = 1 kapag (A+B+Cin) ay odd (1, 3)\n\n";

    cout << "  Ibig sabihin, ang decode ay dapat:\n";
    cout << "  sum = (A + B + Cin) % 2\n";
    cout << "  Hindi: (φ - avg) / (φ - φ⁻¹) % 2\n\n";

    cout << "  KASI:\n";
    cout << "  φ + φ⁻¹ + φ = 3.854 → normalized = ? → sum = ?\n";
    cout << "  Hindi ito simpleng φ o φ⁻¹!\n\n";

    return 0;
}
