// ============================================
// φ-1M ARBITRARY FIXED — WITH WRAP-AROUND
// I-wrap ang frac sa [0,1) at i-carry sa n
// bago ang reconstruction sa dulo
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
    cout << "  φ-1M ARBITRARY FIXED\n";
    cout << "========================================\n\n";

    CCParams<CryptoContextCKKSRNS> parameters;
    parameters.SetMultiplicativeDepth(2);
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
    for (int i = 2; i <= 200; i++) {
        fib.push_back(fib[i-1] + fib[i-2]);
    }

    cout << "  ✅ CKKS initialized (depth 2, modsize 59, 8 slots)\n\n";

    auto encrypt_hg = [&](double x) {
        double log_phi_x = log(x) / LN_PHI;
        double n_val = floor(log_phi_x);
        double frac = log_phi_x - n_val;
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

    auto decrypt_hg = [&](const Ciphertext<DCRTPoly>& ct) {
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

    cout << "========================================\n";
    cout << "  1M ARBITRARY — WITH WRAP-AROUND\n";
    cout << "========================================\n\n";

    int N = 1000000;

    auto ct_state = encrypt_hg(1.0);
    auto ct_two = encrypt_hg(2.0);
    auto ct_three = encrypt_hg(3.0);
    auto ct_one = encrypt_hg(1.0);

    cout << "  Operations: " << N << "\n";
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

    auto final_vals = decrypt_hg(ct_state);

    cout << "  ✅ 1M mixed operations complete!\n";
    cout << "  Time: " << time / 1000.0 << " seconds\n";
    cout << "  Level: " << ct_state->GetLevel() << "\n\n";

    // ============================================
    // WRAP-AROUND + RECONSTRUCTION
    // ============================================

    cout << "========================================\n";
    cout << "  WRAP-AROUND + RECONSTRUCTION\n";
    cout << "========================================\n\n";

    double n_raw = final_vals[1];
    double frac_raw = final_vals[2];

    // I-wrap ang frac sa [0, 1)
    double frac_wrapped = fmod(frac_raw, 1.0);
    double n_wrapped = n_raw + floor(frac_raw);

    cout << "  n raw: " << n_raw << "\n";
    cout << "  frac raw: " << frac_raw << "\n";
    cout << "  frac wrapped: " << frac_wrapped << "\n";
    cout << "  n wrapped: " << n_wrapped << "\n\n";

    // Reconstruct mula sa wrapped values
    int n = (int)n_wrapped;
    double phi_frac = pow(PHI, frac_wrapped);
    double phi_n = fib[n-1] + fib[n] * PHI;
    double reconstructed = phi_n * phi_frac;
    double mod_phi = fmod(reconstructed, PHI);

    cout << "  φ^n = " << phi_n << "\n";
    cout << "  φ^frac = " << phi_frac << "\n";
    cout << "  Reconstructed: " << reconstructed << "\n";
    cout << "  Mod φ: " << mod_phi << "\n\n";

    // Plaintext verification
    double expected = 1.0;
    for (int i = 0; i < N; i++) {
        switch (i % 4) {
            case 0: expected *= 2.0; break;
            case 1: expected /= 2.0; break;
            case 2: expected += 3.0; break;
            case 3: expected -= 1.0; break;
        }
    }
    double expected_mod_phi = fmod(expected, PHI);

    cout << "  Expected raw: " << expected << "\n";
    cout << "  Expected mod φ: " << expected_mod_phi << "\n\n";
    cout << "  Match: " << (abs(mod_phi - expected_mod_phi) < 0.01 ? "✅" : "❌") << "\n\n";

    return 0;
}
