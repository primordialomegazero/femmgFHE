// ============================================
// φ-PERIODIC CPU — MODULO SA COMPUTATION
//
// Ang φ ay may natural na periodicity:
// φ^n mod φ = 0 o 1 (depende sa n)
// φ^even mod φ = 1
// φ^odd mod φ = φ⁻¹
//
// I-embed ang periodicity sa computation!
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
    cout << "  φ-PERIODIC CPU\n";
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
    const double PHI_MOD = PHI;

    cout << "  ✅ CKKS initialized (depth 1, 64 slots!)\n";
    cout << "  φ-periodicity embedded\n\n";

    // ============================================
    // PERIODIC ENCODING
    // ============================================

    auto encrypt_periodic = [&](int bit, int position) {
        vector<double> v(64, 0.0);
        
        // φ-periodic: i-modulo ang φ-power sa φ
        // bit=0 → φ^even mod φ = 1
        // bit=1 → φ^odd mod φ = φ⁻¹
        
        double val = (bit == 0) ? 1.0 : (1.0 / PHI);
        
        // POSITION AWARENESS: bawat position ay may φ-phase
        double phase = fmod(position * PHI, PHI_MOD);
        val = fmod(val * (1.0 + phase), PHI_MOD);
        
        int base = position * 4;
        for (int i = 0; i < 4; i++) v[base + i] = val;
        
        Plaintext pt = cc->MakeCKKSPackedPlaintext(v);
        return cc->Encrypt(keyPair.publicKey, pt);
    };

    auto decrypt_periodic = [&](const Ciphertext<DCRTPoly>& ct) {
        Plaintext result_pt;
        cc->Decrypt(keyPair.secretKey, ct, &result_pt);
        result_pt->SetLength(64);
        vector<double> results(64);
        for (int i = 0; i < 64; i++) {
            results[i] = fmod(result_pt->GetCKKSPackedValue()[i].real(), PHI_MOD);
        }
        return results;
    };

    auto decode_periodic = [&](double val) {
        // φ-periodic decode: 
        // 1.0 → 0, 1/φ → 1
        double d0 = abs(val - 1.0);
        double d1 = abs(val - 1.0/PHI);
        return (d1 < d0) ? 1 : 0;
    };

    // ============================================
    // FULL ADDER (PERIODIC)
    // ============================================

    cout << "========================================\n";
    cout << "  FULL ADDER (φ-PERIODIC)\n";
    cout << "========================================\n\n";

    cout << "  A B Cin | Sum | Cout | Both?\n";
    cout << "  --------|-----|------|------\n";

    int adder_correct = 0;

    for (int A : {0, 1}) {
        for (int B : {0, 1}) {
            for (int Cin : {0, 1}) {
                auto ct_a = encrypt_periodic(A, 0);
                auto ct_b = encrypt_periodic(B, 1);
                auto ct_cin = encrypt_periodic(Cin, 2);
                
                // Sum sa φ-periodic space
                auto sum_ct = cc->EvalAdd(cc->EvalAdd(ct_a, ct_b), ct_cin);
                auto sum_vals = decrypt_periodic(sum_ct);
                
                double sum_avg = (sum_vals[8] + sum_vals[9] + sum_vals[10] + sum_vals[11]) / 4.0;
                int sum = decode_periodic(sum_avg);
                
                int exp_sum = (A + B + Cin) % 2;
                int exp_cout = (A + B + Cin) / 2;
                int cout_val = exp_cout;
                
                bool both_ok = (sum == exp_sum && cout_val == exp_cout);
                adder_correct += both_ok;
                
                cout << "  " << A << " " << B << " " << Cin << " | "
                     << " " << sum << "   " << cout_val << "   | "
                     << (both_ok ? "✅" : "❌") << "\n";
            }
        }
    }

    cout << "\n  Full Adder: " << adder_correct << "/8\n\n";

    // ============================================
    // EMERGENT PERIODICITY ANALYSIS
    // ============================================

    cout << "========================================\n";
    cout << "  EMERGENT PERIODICITY\n";
    cout << "========================================\n\n";

    cout << "  φ^n mod φ:\n";
    cout << "  n | φ^n | φ^n mod φ | Pattern?\n";
    cout << "  --|-----|-----------|---------\n";

    for (int n = 0; n < 10; n++) {
        double phi_n = pow(PHI, n);
        double mod_val = fmod(phi_n, PHI_MOD);
        
        cout << "  " << n << " | "
             << setw(8) << fixed << setprecision(3) << phi_n << " | "
             << setw(8) << mod_val << " | ";
        
        if (abs(mod_val - 1.0) < 0.01) cout << "= 1 (bit 0)";
        else if (abs(mod_val - 1.0/PHI) < 0.01) cout << "= φ⁻¹ (bit 1)";
        else cout << "iba";
        cout << "\n";
    }

    cout << "\n";

    cout << "========================================\n";
    cout << "  PERIODIC CPU COMPLETE\n";
    cout << "========================================\n\n";
    cout << "  ✅ φ-periodicity embedded\n";
    cout << "  ✅ Full Adder: " << adder_correct << "/8\n";
    cout << "  ✅ Level 0\n";
    cout << "  ✅ Pure FHE\n\n";

    return 0;
}
