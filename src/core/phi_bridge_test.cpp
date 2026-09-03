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

    auto encrypt_pair = [&](int n) {
        vector<double> v(2, 0.0);
        v[0] = fmod((double)fib[n], PHI);
        v[1] = fmod((double)fib[n-1], PHI);
        Plaintext pt = cc->MakeCKKSPackedPlaintext(v);
        return cc->Encrypt(keyPair.publicKey, pt);
    };

    auto decrypt_pair = [&](const Ciphertext<DCRTPoly>& ct) {
        Plaintext pt;
        cc->Decrypt(keyPair.secretKey, ct, &pt);
        pt->SetLength(2);
        auto r = pt->GetCKKSPackedValue();
        return vector<double>{r[0].real(), r[1].real()};
    };

    cout << "=== φ-BRIDGE TEST ===\n\n";
    cout << "  n | Slot0 (F_n mod φ) | Slot1 (F_{n-1} mod φ) | Slot0/Slot1 | φ?\n";
    cout << "  --|-------------------|------------------------|-------------|----\n";

    for (int n : {3, 4, 5, 6, 7, 8, 9, 10, 15, 20}) {
        auto ct = encrypt_pair(n);
        auto vals = decrypt_pair(ct);
        double ratio = vals[0] / vals[1];
        double diff = abs(ratio - PHI);
        cout << "  " << setw(2) << n << " | "
             << setw(17) << fixed << setprecision(4) << vals[0] << " | "
             << setw(22) << vals[1] << " | "
             << setw(11) << ratio << " | "
             << (diff < 0.05 ? "✅" : "❌") << "\n";
    }

    cout << "\n=== KEY QUESTION ===\n";
    cout << "  Kung ang Slot0/Slot1 ≈ φ, ito ba ay constant?\n";
    cout << "  Kung oo, ito na ang correction bridge.\n\n";

    return 0;
}
