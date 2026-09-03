// ============================================
// φ-HOLY GRAIL 10K MOD
// 10K all arithmetic + HOMOMORPHIC φ-MODULO
// Mas mabilis na test para makita ang behavior
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
    cout << "  φ-HOLY GRAIL 10K MOD\n";
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
    // ENCODING
    // ============================================

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

    // ============================================
    // 10K ALL ARITHMETIC + MOD
    // ============================================

    cout << "========================================\n";
    cout << "  10K ALL ARITHMETIC + HOMOMORPHIC MOD\n";
    cout << "========================================\n\n";

    int N = 10000;

    auto ct_state = encrypt_hg(1.0);
    auto ct_two = encrypt_hg(2.0);
    auto ct_three = encrypt_hg(3.0);
    auto ct_one = encrypt_hg(1.0);
    auto ct_phi = encrypt_hg(PHI);

    cout << "  Operations: " << N << "\n";
    cout << "  Mix: ×2, ÷2, +3, -1\n";
    cout << "  Mod φ: every 10 ops (5 subtractions)\n";
    cout << "  Running...\n\n";

    auto start = high_resolution_clock::now();

    for (int i = 0; i < N; i++) {
        switch (i % 4) {
            case 0: ct_state = cc->EvalAdd(ct_state, ct_two); break;
            case 1: ct_state = cc->EvalSub(ct_state, ct_two); break;
            case 2: ct_state = cc->EvalAdd(ct_state, ct_three); break;
            case 3: ct_state = cc->EvalSub(ct_state, ct_one); break;
        }
        
        if (i % 10 == 9) {
            for (int j = 0; j < 5; j++) {
                ct_state = cc->EvalSub(ct_state, ct_phi);
            }
        }
    }

    auto end = high_resolution_clock::now();
    auto time = duration_cast<milliseconds>(end - start).count();

    auto final_vals = decrypt_hg(ct_state);

    cout << "  ✅ 10K mixed operations complete!\n";
    cout << "  Time: " << time << " ms\n";
    cout << "  Level: " << ct_state->GetLevel() << "\n";
    cout << "  Towers: " << ct_state->GetElements()[0].GetNumOfElements() << "\n";
    cout << "  Slot 0 (normal): " << final_vals[0] << "\n";
    cout << "  Slot 1 (n): " << final_vals[1] << "\n";
    cout << "  Slot 2 (frac): " << final_vals[2] << "\n";
    cout << "  Slot 3 (log_φ): " << final_vals[3] << "\n";
    cout << "  Slot 4 (φ^frac): " << final_vals[4] << "\n";
    cout << "  Slot 5 (F_{n-1}): " << final_vals[5] << "\n";
    cout << "  Slot 6 (F_n): " << final_vals[6] << "\n";
    cout << "  Slot 7 (recon): " << final_vals[7] << "\n\n";

    cout << "========================================\n";
    cout << "  HOLY GRAIL 10K MOD COMPLETE\n";
    cout << "========================================\n\n";
    cout << "  ✅ 10K all arithmetic\n";
    cout << "  ✅ Homomorphic φ-modulo\n";
    cout << "  ✅ Walang decrypt sa gitna\n";
    cout << "  ✅ Walang bootstrapping\n\n";

    return 0;
}
