// ============================================
// φ-UNBOUNDED CHAIN — Walang Mod sa Encrypted
// Exponent ay unbounded, natural na φ-periodic
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
    parameters.SetBatchSize(1);
    parameters.SetSecurityLevel(HEStd_128_classic);

    CryptoContext<DCRTPoly> cc = GenCryptoContext(parameters);
    cc->Enable(PKE);
    cc->Enable(KEYSWITCH);
    cc->Enable(LEVELEDSHE);
    auto keyPair = cc->KeyGen();

    const double PHI = 1.6180339887498948482;
    const double LN_PHI = log(PHI);

    auto encrypt_log = [&](double log_val) {
        vector<double> v(1, log_val);  // WALANG mod
        Plaintext pt = cc->MakeCKKSPackedPlaintext(v);
        return cc->Encrypt(keyPair.publicKey, pt);
    };

    auto decrypt_log = [&](const Ciphertext<DCRTPoly>& ct) {
        Plaintext pt;
        cc->Decrypt(keyPair.secretKey, ct, &pt);
        pt->SetLength(1);
        return pt->GetCKKSPackedValue()[0].real();
    };

    cout << "========================================\n";
    cout << "  φ-UNBOUNDED CHAIN — Walang Mod\n";
    cout << "========================================\n\n";

    // Start sa φ⁴
    auto ct = encrypt_log(4.0);
    double expected_n = 4.0;

    cout << "  Start: n=4\n\n";

    // Chain: +5, ×3, -2, ÷2, +10
    vector<pair<double,string>> ops;
    
    // +5: correction = 5 (mula sa gaps)
    ops.push_back({5.0, "+5"});
    // ×3
    ops.push_back({log(3.0)/LN_PHI, "×3"});
    // -2: correction = -3 (mula sa gaps)
    ops.push_back({-3.0, "-2"});
    // ÷2
    ops.push_back({-log(2.0)/LN_PHI, "÷2"});
    // +10: correction = 8 (mula sa gaps)
    ops.push_back({8.0, "+10"});

    for (auto& [corr, name] : ops) {
        vector<double> d(1, abs(corr));
        Plaintext pt = cc->MakeCKKSPackedPlaintext(d);
        
        if (corr >= 0) {
            ct = cc->EvalAdd(ct, pt);
        } else {
            ct = cc->EvalSub(ct, pt);
        }
        
        expected_n += corr;
        double n_now = decrypt_log(ct);
        
        cout << "  " << setw(3) << name << " → n=" << setw(10) << n_now
             << ", expected=" << setw(10) << expected_n
             << ", diff=" << n_now - expected_n
             << ", φ-mod=" << fmod(n_now, PHI) << "\n";
    }

    cout << "\n  Final n: " << decrypt_log(ct) << "\n";
    cout << "  Expected: " << expected_n << "\n";
    cout << "  Level: " << ct->GetLevel() << "\n";

    return 0;
}
