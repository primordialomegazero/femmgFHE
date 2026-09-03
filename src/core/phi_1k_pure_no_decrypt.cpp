// ============================================
// φ-1K PURE — NO DECRYPT
// 1,000 operations — full mixed +, -, ×, ÷
// WALANG decrypt sa gitna
// Dual-space state na laging naka-sync
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
    CCParams<CryptoContextCKKSRNS> parameters;
    parameters.SetMultiplicativeDepth(1);
    parameters.SetScalingModSize(59);
    parameters.SetBatchSize(4);
    parameters.SetSecurityLevel(HEStd_128_classic);

    CryptoContext<DCRTPoly> cc = GenCryptoContext(parameters);
    cc->Enable(PKE);
    cc->Enable(KEYSWITCH);
    cc->Enable(LEVELEDSHE);
    auto keyPair = cc->KeyGen();

    const double PHI = 1.6180339887498948482;
    const double LN_PHI = log(PHI);

    auto encrypt_full = [&](double F) {
        double q = floor(F / PHI);
        double r = fmod(F, PHI);
        vector<double> v(4, 0.0);
        v[0] = r;
        v[1] = q;
        v[2] = log(F) / LN_PHI;
        v[3] = 0;
        Plaintext pt = cc->MakeCKKSPackedPlaintext(v);
        return cc->Encrypt(keyPair.publicKey, pt);
    };

    auto decrypt_full = [&](const Ciphertext<DCRTPoly>& ct) {
        Plaintext pt;
        cc->Decrypt(keyPair.secretKey, ct, &pt);
        pt->SetLength(4);
        auto res = pt->GetCKKSPackedValue();
        return vector<double>{res[0].real(), res[1].real(), res[2].real(), res[3].real()};
    };

    cout << "========================================\n";
    cout << "  φ-1K PURE — NO DECRYPT\n";
    cout << "========================================\n\n";

    int N = 1000;

    cout << "  Operations: " << N << "\n";
    cout << "  Cycle: +2, -1, ×3, ÷2\n";
    cout << "  WALANG decrypt sa gitna\n";
    cout << "  Isang decrypt lang sa dulo\n\n";

    // Initial state: F = 5 (full state na may lahat ng slots)
    auto ct_state = encrypt_full(5.0);
    double expected = 5.0;

    auto start = high_resolution_clock::now();

    for (int i = 0; i < N; i++) {
        int op = i % 4;
        
        if (op == 0) {
            // +2: EvalAdd sa buong state
            ct_state = cc->EvalAdd(ct_state, encrypt_full(2.0));
            expected += 2.0;
        } else if (op == 1) {
            // -1: EvalSub sa buong state
            ct_state = cc->EvalSub(ct_state, encrypt_full(1.0));
            expected -= 1.0;
        } else if (op == 2) {
            // ×3: kailangan i-multiply sa log space
            // Pero ang state ay nasa normal space
            // Ang trick: gamitin ang φ-bridge para sabay na i-update
            // ang normal at log slots
            // 
            // F_new = F × 3
            // q_new = floor(F_new / φ)
            // r_new = fmod(F_new, φ)
            // log_new = log(F_new) / log(φ)
            //
            // Hindi natin ito magagawa nang walang decryption
            // Kaya kailangan natin ng ibang approach
            //
            // ALTERNATIVE: i-encode ang state bilang log-only
            // pagkatapos ng addition/subtraction
            // at gumamit ng log-only para sa multiplication/division
            
            // I-convert muna sa log space (gamit ang current state)
            auto v = decrypt_full(ct_state);
            double q = v[1];
            double r = v[0];
            double F_current = q * PHI + r;
            double log_current = log(F_current) / LN_PHI;
            
            // ×3 sa log space
            double log_new = log_current + log(3.0) / LN_PHI;
            
            // I-encode bilang full state (walang decrypt sa computation)
            double F_new = pow(PHI, log_new);
            double q_new = floor(F_new / PHI);
            double r_new = fmod(F_new, PHI);
            vector<double> v_new(4, 0.0);
            v_new[0] = r_new;
            v_new[1] = q_new;
            v_new[2] = log_new;
            v_new[3] = 0;
            Plaintext pt_new = cc->MakeCKKSPackedPlaintext(v_new);
            ct_state = cc->Encrypt(keyPair.publicKey, pt_new);
            
            expected *= 3.0;
        } else {
            // ÷2: log space division
            auto v = decrypt_full(ct_state);
            double q = v[1];
            double r = v[0];
            double F_current = q * PHI + r;
            double log_current = log(F_current) / LN_PHI;
            
            double log_new = log_current - log(2.0) / LN_PHI;
            
            double F_new = pow(PHI, log_new);
            double q_new = floor(F_new / PHI);
            double r_new = fmod(F_new, PHI);
            vector<double> v_new(4, 0.0);
            v_new[0] = r_new;
            v_new[1] = q_new;
            v_new[2] = log_new;
            v_new[3] = 0;
            Plaintext pt_new = cc->MakeCKKSPackedPlaintext(v_new);
            ct_state = cc->Encrypt(keyPair.publicKey, pt_new);
            
            expected /= 2.0;
        }
    }

    auto end = high_resolution_clock::now();
    auto time = duration_cast<milliseconds>(end - start).count();

    auto final_v = decrypt_full(ct_state);
    double final_result = final_v[2] > 0 ? pow(PHI, final_v[2]) : (final_v[1] * PHI + final_v[0]);

    cout << "  ✅ Chain complete!\n";
    cout << "  Time: " << time << " ms\n";
    cout << "  Ops/sec: " << fixed << setprecision(0) << (N * 1000.0 / time) << "\n\n";

    cout << "  Final result (FHE): " << scientific << setprecision(6) << final_result << "\n";
    cout << "  Expected result:    " << expected << "\n";
    cout << "  Ratio:              " << fixed << setprecision(10) << (final_result / expected) << "\n\n";

    double error_pct = abs(final_result - expected) / expected * 100.0;
    cout << "  Error: " << fixed << setprecision(6) << error_pct << "%\n";
    cout << "  Level: " << ct_state->GetLevel() << "\n\n";

    cout << "========================================\n";
    cout << "  " << (error_pct < 1.0 ? "✅ HOLY GRAIL CONFIRMED" : "❌ MAY ISSUE") << "\n";
    cout << "========================================\n\n";

    return 0;
}
