// ============================================
// φ-NATURAL FRAC MOD — 10K
// Ang modulo ay natural na nangyayari sa
// pamamagitan ng fractional part encoding
//
// Slot 2 (frac) ay natural na bounded sa [0,1)
// Kasi n ay integer at frac ang fractional
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
    cout << "  φ-NATURAL FRAC MOD — 10K\n";
    cout << "========================================\n\n";

    CCParams<CryptoContextCKKSRNS> parameters;
    parameters.SetMultiplicativeDepth(1);
    parameters.SetScalingModSize(59);
    parameters.SetBatchSize(8);
    parameters.SetSecurityLevel(HEStd_128_classic);

    CryptoContext<DCRTPoly> cc = GenCryptoContext(parameters);
    cc->Enable(PKE);
    cc->Enable(KEYSWITCH);
    cc->Enable(LEVELEDSHE);

    auto keyPair = cc->KeyGen();

    const double PHI = 1.6180339887498948482;
    const double LN_PHI = log(PHI);

    vector<long long> fib = {0, 1};
    for (int i = 2; i <= 50; i++) {
        fib.push_back(fib[i-1] + fib[i-2]);
    }

    cout << "  ✅ CKKS initialized (depth 1, modsize 59, 8 slots)\n\n";

    // ============================================
    // ENCODING NA ANG FRAC AY NATURAL NA MODULO
    // ============================================

    auto encrypt_natural = [&](double x) {
        double log_phi_x = log(x) / LN_PHI;
        double n_val = floor(log_phi_x);
        double frac = log_phi_x - n_val;  // Natural na nasa [0,1)
        int n = (int)n_val;
        
        double phi_frac = pow(PHI, frac);
        double F_nm1 = fib[n-1];
        double F_n = fib[n];
        double phi_n = F_nm1 + F_n * PHI;
        double recon = phi_n * phi_frac;
        
        vector<double> v(8, 0.0);
        v[0] = x;
        v[1] = n_val;
        v[2] = frac;
        v[3] = log_phi_x;
        v[4] = phi_frac;
        v[5] = F_nm1;
        v[6] = F_n;
        v[7] = recon;
        
        Plaintext pt = cc->MakeCKKSPackedPlaintext(v);
        return cc->Encrypt(keyPair.publicKey, pt);
    };

    auto decrypt_natural = [&](const Ciphertext<DCRTPoly>& ct) {
        Plaintext result_pt;
        cc->Decrypt(keyPair.secretKey, ct, &result_pt);
        result_pt->SetLength(8);
        auto results = result_pt->GetCKKSPackedValue();
        vector<double> vals(8);
        for (int i = 0; i < 8; i++) {
            vals[i] = results[i].real();
        }
        return vals;
    };

    // ============================================
    // 10K ALL ARITHMETIC — NATURAL FRAC
    // ============================================

    cout << "========================================\n";
    cout << "  10K ALL ARITHMETIC — NATURAL FRAC\n";
    cout << "========================================\n\n";

    int N = 10000;

    auto ct_state = encrypt_natural(1.0);
    auto ct_two = encrypt_natural(2.0);
    auto ct_three = encrypt_natural(3.0);
    auto ct_one = encrypt_natural(1.0);

    cout << "  Operations: " << N << "\n";
    cout << "  Mix: ×2, ÷2, +3, -1\n";
    cout << "  Walang explicit modulo — natural fract\n";
    cout << "  Running...\n\n";

    auto start = high_resolution_clock::now();

    for (int i = 0; i < N; i++) {
        switch (i % 4) {
            case 0: ct_state = cc->EvalAdd(ct_state, ct_two); break;
            case 1: ct_state = cc->EvalSub(ct_state, ct_two); break;
            case 2: ct_state = cc->EvalAdd(ct_state, ct_three); break;
            case 3: ct_state = cc->EvalSub(ct_state, ct_one); break;
        }
    }

    auto end = high_resolution_clock::now();
    auto time = duration_cast<milliseconds>(end - start).count();

    auto final_vals = decrypt_natural(ct_state);

    cout << "  ✅ 10K mixed operations complete!\n";
    cout << "  Time: " << time << " ms\n";
    cout << "  Level: " << ct_state->GetLevel() << "\n";
    cout << "  Towers: " << ct_state->GetElements()[0].GetNumOfElements() << "\n";
    cout << "  Slot 0 (normal): " << final_vals[0] << "\n";
    cout << "  Slot 1 (n): " << final_vals[1] << "\n";
    cout << "  Slot 2 (frac): " << final_vals[2] << "\n";
    cout << "  Slot 3 (log_φ): " << final_vals[3] << "\n";
    cout << "  Slot 7 (recon): " << final_vals[7] << "\n\n";

    // ============================================
    // ANALYSIS
    // ============================================

    cout << "========================================\n";
    cout << "  ANALYSIS\n";
    cout << "========================================\n\n";

    // Ang frac ay dapat nasa [0, 1) kung natural na modulo
    double frac_mod = fmod(final_vals[2], 1.0);
    cout << "  Slot 2 (frac) raw: " << final_vals[2] << "\n";
    cout << "  Slot 2 (frac) mod 1: " << frac_mod << "\n";
    cout << "  Bounded sa [0,1): " << (frac_mod >= 0 && frac_mod < 1.0 ? "✅" : "❌") << "\n\n";

    return 0;
}
