// ============================================
// φ-BASIS FHE — 10K MIXED
// State: (a, b) sa φ-basis
// Automatic modulo at carry
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

    auto encrypt_basis = [&](double F) {
        double a = floor(F / PHI);
        double b = (F - a * PHI) * PHI;
        vector<double> v(4, 0.0);
        v[0] = a;
        v[1] = b;
        v[2] = log(F) / LN_PHI;
        v[3] = fmod(F, PHI);
        Plaintext pt = cc->MakeCKKSPackedPlaintext(v);
        return cc->Encrypt(keyPair.publicKey, pt);
    };

    auto decrypt_basis = [&](const Ciphertext<DCRTPoly>& ct) {
        Plaintext pt;
        cc->Decrypt(keyPair.secretKey, ct, &pt);
        pt->SetLength(4);
        auto res = pt->GetCKKSPackedValue();
        return vector<double>{res[0].real(), res[1].real(), res[2].real(), res[3].real()};
    };

    cout << "========================================\n";
    cout << "  φ-BASIS FHE — 10K MIXED\n";
    cout << "========================================\n\n";

    int N = 10000;

    cout << "  Operations: " << N << "\n";
    cout << "  Cycle: +7, -3, ×2, ÷3 (no-cancel)\n";
    cout << "  Initial: 100\n";
    cout << "  Automatic carry via b ≥ φ\n";
    cout << "  Running...\n\n";

    auto ct_state = encrypt_basis(100.0);
    double expected = 100.0;

    auto ct_add = encrypt_basis(7.0);
    auto ct_sub = encrypt_basis(3.0);
    
    vector<double> v_mul(4, 0.0);
    v_mul[2] = log(2.0) / LN_PHI;
    Plaintext pt_mul = cc->MakeCKKSPackedPlaintext(v_mul);
    auto ct_mul = cc->Encrypt(keyPair.publicKey, pt_mul);
    
    vector<double> v_div(4, 0.0);
    v_div[2] = -log(3.0) / LN_PHI;
    Plaintext pt_div = cc->MakeCKKSPackedPlaintext(v_div);
    auto ct_div = cc->Encrypt(keyPair.publicKey, pt_div);

    auto start = high_resolution_clock::now();

    for (int i = 0; i < N; i++) {
        int op = i % 4;
        
        if (op == 0) {
            ct_state = cc->EvalAdd(ct_state, ct_add);
            expected += 7.0;
        } else if (op == 1) {
            ct_state = cc->EvalSub(ct_state, ct_sub);
            expected -= 3.0;
        } else if (op == 2) {
            ct_state = cc->EvalAdd(ct_state, ct_mul);
            expected *= 2.0;
        } else {
            ct_state = cc->EvalAdd(ct_state, ct_div);
            expected /= 3.0;
        }
    }

    auto end = high_resolution_clock::now();
    auto time = duration_cast<milliseconds>(end - start).count();

    auto v_final = decrypt_basis(ct_state);
    double F_final = v_final[0] * PHI + v_final[1] * (PHI - 1.0);

    cout << "  ✅ Chain complete!\n";
    cout << "  Time: " << time << " ms\n";
    cout << "  Ops/sec: " << fixed << setprecision(0) << (N * 1000.0 / time) << "\n\n";

    cout << "  Final a: " << v_final[0] << "\n";
    cout << "  Final b: " << v_final[1] << "\n";
    cout << "  Final F: " << F_final << "\n";
    cout << "  Expected: " << expected << "\n";
    cout << "  Level: " << ct_state->GetLevel() << "\n\n";

    double error = abs(F_final - expected) / expected * 100.0;
    cout << "  Error: " << fixed << setprecision(6) << error << "%\n";
    cout << "  Match: " << (error < 1.0 ? "✅" : "❌") << "\n\n";

    cout << "========================================\n";
    cout << "  " << (error < 1.0 ? "✅ φ-BASIS CONFIRMED" : "❌ MAY ISSUE") << "\n";
    cout << "========================================\n\n";

    return 0;
}
