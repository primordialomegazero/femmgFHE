// ============================================
// φ-DUAL CHANNEL FHE
// (r_n, q_n) bilang kumpletong state
// EvalAdd sa parehong channels
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
    parameters.SetBatchSize(2);
    parameters.SetSecurityLevel(HEStd_128_classic);

    CryptoContext<DCRTPoly> cc = GenCryptoContext(parameters);
    cc->Enable(PKE);
    cc->Enable(KEYSWITCH);
    cc->Enable(LEVELEDSHE);
    auto keyPair = cc->KeyGen();

    const double PHI = 1.6180339887498948482;
    vector<long long> fib = {0, 1};
    for (int i = 2; i <= 100; i++) fib.push_back(fib[i-1] + fib[i-2]);

    auto encrypt_dual = [&](int n) {
        double F = (double)fib[n];
        double q = floor(F / PHI);
        double r = fmod(F, PHI);
        vector<double> v(2, 0.0);
        v[0] = r;  // fractional part
        v[1] = q;  // wrap count
        Plaintext pt = cc->MakeCKKSPackedPlaintext(v);
        return cc->Encrypt(keyPair.publicKey, pt);
    };

    auto decrypt_dual = [&](const Ciphertext<DCRTPoly>& ct) {
        Plaintext pt;
        cc->Decrypt(keyPair.secretKey, ct, &pt);
        pt->SetLength(2);
        auto res = pt->GetCKKSPackedValue();
        return vector<double>{res[0].real(), res[1].real()};
    };

    cout << "=== φ-DUAL CHANNEL FHE ===\n\n";
    cout << "  Addition sa dual-channel state\n\n";

    cout << "  a+b | r_sum | q_sum | Recon (q×φ + r) | Expected | Match?\n";
    cout << "  ----|-------|-------|------------------|----------|-------\n";

    for (int a : {3, 4, 5}) {
        for (int b : {a, a+1, a+2}) {
            auto ct_a = encrypt_dual(a);
            auto ct_b = encrypt_dual(b);
            auto ct_sum = cc->EvalAdd(ct_a, ct_b);
            auto vals = decrypt_dual(ct_sum);

            double recon = vals[1] * PHI + vals[0];
            double expected = (double)(fib[a] + fib[b]);
            bool match = abs(recon - expected) < 0.5;

            cout << "  " << setw(3) << a << "+" << b << " | "
                 << setw(5) << fixed << setprecision(2) << vals[0] << " | "
                 << setw(5) << vals[1] << " | "
                 << setw(16) << recon << " | "
                 << setw(8) << expected << " | "
                 << (match ? "✅" : "❌") << "\n";
        }
    }

    cout << "\n=== KEY ===\n";
    cout << "  Kung ✅ lahat, ang dual-channel addition ay exact\n";
    cout << "  na walang modulo carry handling\n\n";

    return 0;
}
