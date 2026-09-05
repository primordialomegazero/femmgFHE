// ============================================
// φ-FIB ADD SIMPLE — 10 iterations lang
// Pure EvalAdd + EvalRotate
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
    cc->EvalRotateKeyGen(keyPair.secretKey, {1, -1});

    auto encrypt = [&](double a, double b) {
        vector<double> v = {a, b};
        Plaintext pt = cc->MakeCKKSPackedPlaintext(v);
        return cc->Encrypt(keyPair.publicKey, pt);
    };

    auto decrypt = [&](const Ciphertext<DCRTPoly>& ct) {
        Plaintext pt;
        cc->Decrypt(keyPair.secretKey, ct, &pt);
        pt->SetLength(2);
        auto res = pt->GetCKKSPackedValue();
        return vector<double>{res[0].real(), res[1].real()};
    };

    cout << "=== φ-FIB ADD SIMPLE ===\n\n";
    cout << "Start: (F_2=1, F_1=1)\n\n";

    auto ct = encrypt(1, 1);

    for (int i = 0; i < 10; i++) {
        auto before = decrypt(ct);
        cout << "Step " << i << ": (" << before[0] << ", " << before[1] << ")\n";
        
        // EvalRotate by 1: swap slots
        auto rotated = cc->EvalRotate(ct, 1);
        ct = cc->EvalAdd(ct, rotated);
    }

    auto after = decrypt(ct);
    cout << "Final: (" << after[0] << ", " << after[1] << ")\n";
    cout << "Level: " << ct->GetLevel() << "\n";

    return 0;
}
