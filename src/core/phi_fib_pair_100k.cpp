// ============================================
// φ-FIB PAIR 100K — FULL MIXED
// 100,000 operations: +F_3, -F_1, ×φ, ÷φ
// Isang encrypt, isang decrypt
// WALANG re-encryption, WALANG bootstrapping
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

    vector<long long> fib = {0, 1};
    for (int i = 2; i <= 200; i++) fib.push_back(fib[i-1] + fib[i-2]);

    auto encrypt_state = [&](int n) {
        vector<double> v(4, 0.0);
        v[0] = fib[n];
        v[1] = fib[n-1];
        v[2] = log((double)fib[n]) / LN_PHI;
        v[3] = n;
        Plaintext pt = cc->MakeCKKSPackedPlaintext(v);
        return cc->Encrypt(keyPair.publicKey, pt);
    };

    auto decrypt_state = [&](const Ciphertext<DCRTPoly>& ct) {
        Plaintext pt;
        cc->Decrypt(keyPair.secretKey, ct, &pt);
        pt->SetLength(4);
        auto res = pt->GetCKKSPackedValue();
        return vector<double>{res[0].real(), res[1].real(), res[2].real(), res[3].real()};
    };

    cout << "========================================\n";
    cout << "  φ-FIB PAIR 100K — FULL MIXED\n";
    cout << "========================================\n\n";

    int N = 100000;

    cout << "  Operations: " << N << "\n";
    cout << "  Cycle: +F_3, -F_1, ×φ, ÷φ\n";
    cout << "  Net per cycle: +1 sa Slot 0\n";
    cout << "  Isang encrypt, isang decrypt\n";
    cout << "  WALANG re-encryption\n";
    cout << "  Running...\n\n";

    auto ct_state = encrypt_state(10);  // F_10 = 55

    auto ct_add = encrypt_state(3);   // +F_3 = +2
    auto ct_sub = encrypt_state(1);   // -F_1 = -1

    vector<double> v_mul(4, 0.0);
    v_mul[2] = 1.0;
    Plaintext pt_mul = cc->MakeCKKSPackedPlaintext(v_mul);
    auto ct_mul = cc->Encrypt(keyPair.publicKey, pt_mul);

    vector<double> v_div(4, 0.0);
    v_div[2] = -1.0;
    Plaintext pt_div = cc->MakeCKKSPackedPlaintext(v_div);
    auto ct_div = cc->Encrypt(keyPair.publicKey, pt_div);

    auto start = high_resolution_clock::now();

    for (int i = 0; i < N; i++) {
        int op = i % 4;
        
        if (op == 0) ct_state = cc->EvalAdd(ct_state, ct_add);
        else if (op == 1) ct_state = cc->EvalSub(ct_state, ct_sub);
        else if (op == 2) ct_state = cc->EvalAdd(ct_state, ct_mul);
        else ct_state = cc->EvalAdd(ct_state, ct_div);
    }

    auto end = high_resolution_clock::now();
    auto time = duration_cast<milliseconds>(end - start).count();

    auto v_final = decrypt_state(ct_state);

    // Expected: 55 + 25000×(2-1) = 25055
    double expected = 55.0 + (N/4) * (2.0 - 1.0);

    cout << "  ✅ Chain complete!\n";
    cout << "  Time: " << time << " ms\n";
    cout << "  Ops/sec: " << fixed << setprecision(0) << (N * 1000.0 / time) << "\n\n";

    cout << "  Final Slot 0: " << v_final[0] << " (Expected: " << expected << ")\n";
    cout << "  Final Slot 1: " << v_final[1] << "\n";
    cout << "  Final Slot 2: " << v_final[2] << "\n";
    cout << "  Final Slot 3: " << v_final[3] << "\n";
    cout << "  Level: " << ct_state->GetLevel() << "\n\n";

    double error = abs(v_final[0] - expected) / expected * 100.0;
    cout << "  Error: " << fixed << setprecision(6) << error << "%\n";
    cout << "  Match: " << (error < 0.01 ? "✅" : "❌") << "\n\n";

    cout << "========================================\n";
    cout << "  " << (error < 0.01 ? "✅ HOLY GRAIL CONFIRMED — 100K" : "❌ MAY ISSUE") << "\n";
    cout << "========================================\n\n";

    return 0;
}
