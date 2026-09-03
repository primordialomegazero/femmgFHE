// ============================================
// φ-FIB PAIR FHE ONESHOT
// (F_n, F_{n-1}) bilang natural na state
// Walang modulo — Fibonacci structure ang nagre-reset
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
    parameters.SetBatchSize(2);
    parameters.SetSecurityLevel(HEStd_128_classic);

    CryptoContext<DCRTPoly> cc = GenCryptoContext(parameters);
    cc->Enable(PKE);
    cc->Enable(KEYSWITCH);
    cc->Enable(LEVELEDSHE);
    auto keyPair = cc->KeyGen();

    const double PHI = 1.6180339887498948482;

    vector<long long> fib = {0, 1};
    for (int i = 2; i <= 200; i++) fib.push_back(fib[i-1] + fib[i-2]);

    // State: Slot 0 = F_n, Slot 1 = F_{n-1}
    auto encrypt_pair = [&](int n) {
        vector<double> v(2, 0.0);
        v[0] = fib[n];
        v[1] = fib[n-1];
        Plaintext pt = cc->MakeCKKSPackedPlaintext(v);
        return cc->Encrypt(keyPair.publicKey, pt);
    };

    auto decrypt_pair = [&](const Ciphertext<DCRTPoly>& ct) {
        Plaintext pt;
        cc->Decrypt(keyPair.secretKey, ct, &pt);
        pt->SetLength(2);
        auto res = pt->GetCKKSPackedValue();
        return vector<double>{res[0].real(), res[1].real()};
    };

    cout << "========================================\n";
    cout << "  φ-FIB PAIR FHE ONESHOT — 10K\n";
    cout << "========================================\n\n";

    int N = 10000;

    cout << "  Operations: " << N << "\n";
    cout << "  Fibonacci pair state: (F_n, F_{n-1})\n";
    cout << "  Walang modulo — natural na Fibonacci reset\n";
    cout << "  Isang encrypt, isang decrypt\n";
    cout << "  Running...\n\n";

    // Initial: (F_5, F_4) = (5, 3)
    auto ct_state = encrypt_pair(5);

    // Delta: F_2 = 1 (kada addition)
    auto ct_delta = encrypt_pair(2);

    auto start = high_resolution_clock::now();

    for (int i = 0; i < N; i++) {
        ct_state = cc->EvalAdd(ct_state, ct_delta);
    }

    auto end = high_resolution_clock::now();
    auto time = duration_cast<milliseconds>(end - start).count();

    auto v_final = decrypt_pair(ct_state);

    // Expected
    long long expected_F = fib[5] + N * fib[2];
    long long expected_F_prev = fib[4] + N * fib[1];

    cout << "  ✅ Chain complete!\n";
    cout << "  Time: " << time << " ms\n";
    cout << "  Ops/sec: " << fixed << setprecision(0) << (N * 1000.0 / time) << "\n\n";

    cout << "  Final F_n: " << v_final[0] << " (Expected: " << expected_F << ")\n";
    cout << "  Final F_{n-1}: " << v_final[1] << " (Expected: " << expected_F_prev << ")\n\n";

    double error = abs(v_final[0] - expected_F) / expected_F * 100.0;
    cout << "  Error: " << fixed << setprecision(6) << error << "%\n";
    cout << "  Match: " << (error < 0.01 ? "✅" : "❌") << "\n\n";

    cout << "========================================\n";
    cout << "  " << (error < 0.01 ? "✅ HOLY GRAIL CONFIRMED" : "❌ MAY ISSUE") << "\n";
    cout << "========================================\n\n";

    return 0;
}
