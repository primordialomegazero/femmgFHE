// ============================================
// φ-COMPLETE UNIFIED STATE
// 4 channels sa iisang ciphertext:
// Slot 0: r_n (F_n mod φ)
// Slot 1: q_n (wrap count)
// Slot 2: log_φ(F_n)
// Slot 3: n (index)
// ============================================

#include <iostream>
#include <vector>
#include <cmath>
#include <iomanip>
#include "pke/openfhe.h"

using namespace lbcrypto;
using namespace std;

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
    for (int i = 2; i <= 100; i++) fib.push_back(fib[i-1] + fib[i-2]);

    auto encrypt_full = [&](int n) {
        double F = (double)fib[n];
        double q = floor(F / PHI);
        double r = fmod(F, PHI);
        double log_phi_F = log(F) / LN_PHI;
        
        vector<double> v(4, 0.0);
        v[0] = r;
        v[1] = q;
        v[2] = log_phi_F;
        v[3] = n;
        
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

    cout << "=== φ-COMPLETE UNIFIED STATE ===\n\n";

    // ============================================
    // TEST 1: ADDITION (Normal Space)
    // ============================================
    cout << "--- TEST 1: ADDITION (Normal Space) ---\n\n";
    cout << "  a+b | r_sum | q_sum | Recon | Expected | Match?\n";
    cout << "  ----|-------|-------|-------|----------|-------\n";

    for (int a : {3, 5, 7}) {
        for (int b : {a, a+1, a+2}) {
            auto ct_a = encrypt_full(a);
            auto ct_b = encrypt_full(b);
            auto ct_sum = cc->EvalAdd(ct_a, ct_b);
            auto vals = decrypt_full(ct_sum);

            double recon = vals[1] * PHI + vals[0];
            double expected = (double)(fib[a] + fib[b]);
            bool match = abs(recon - expected) < 0.5;

            cout << "  " << setw(3) << a << "+" << b << " | "
                 << setw(5) << fixed << setprecision(2) << vals[0] << " | "
                 << setw(5) << vals[1] << " | "
                 << setw(5) << recon << " | "
                 << setw(8) << expected << " | "
                 << (match ? "✅" : "❌") << "\n";
        }
    }

    // ============================================
    // TEST 2: MULTIPLICATION (Log Space)
    // ============================================
    cout << "\n--- TEST 2: MULTIPLICATION (Log Space) ---\n\n";
    cout << "  a×b | log_sum | φ^(log_sum) | Expected | Match?\n";
    cout << "  ----|---------|--------------|----------|-------\n";

    for (int a : {3, 4, 5}) {
        for (int b : {a, a+1}) {
            auto ct_a = encrypt_full(a);
            auto ct_b = encrypt_full(b);
            auto ct_sum = cc->EvalAdd(ct_a, ct_b);
            auto vals = decrypt_full(ct_sum);

            double product = pow(PHI, vals[2]);
            double expected = (double)(fib[a] * fib[b]);
            bool match = abs(product - expected) < 1.0;

            cout << "  " << setw(3) << a << "×" << b << " | "
                 << setw(7) << fixed << setprecision(3) << vals[2] << " | "
                 << setw(12) << product << " | "
                 << setw(8) << expected << " | "
                 << (match ? "✅" : "❌") << "\n";
        }
    }

    // ============================================
    // TEST 3: INDEX TRACKING
    // ============================================
    cout << "\n--- TEST 3: INDEX TRACKING ---\n\n";
    cout << "  a+b | index_sum | Expected (a+b) | Match?\n";
    cout << "  ----|-----------|----------------|-------\n";

    for (int a : {3, 5}) {
        for (int b : {a, a+1}) {
            auto ct_a = encrypt_full(a);
            auto ct_b = encrypt_full(b);
            auto ct_sum = cc->EvalAdd(ct_a, ct_b);
            auto vals = decrypt_full(ct_sum);

            double expected = a + b;
            bool match = abs(vals[3] - expected) < 0.01;

            cout << "  " << setw(3) << a << "+" << b << " | "
                 << setw(9) << fixed << setprecision(1) << vals[3] << " | "
                 << setw(14) << expected << " | "
                 << (match ? "✅" : "❌") << "\n";
        }
    }

    cout << "\n=== KEY ===\n";
    cout << "  Kung lahat ✅, ang 4-channel state ay kumpleto\n";
    cout << "  na may unified normal at log space\n\n";

    return 0;
}
