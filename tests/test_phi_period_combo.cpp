// φ-PERIOD COMBINATION — 8 STATES
// I-combine ang period-2 at period-3 para sa Rule 110

#include "openfhe.h"
#include <iostream>
#include <vector>
#include <complex>
#include <cmath>

using namespace lbcrypto;

int main() {
    std::cout << "========================================\n";
    std::cout << "  φ-PERIOD COMBINATION\n";
    std::cout << "  Period-2 × Period-3 = Period-6?\n";
    std::cout << "========================================\n\n";

    const double phi = 1.6180339887498948482;
    const double phi_sq = phi * phi;
    const double phi_cu = phi * phi * phi;

    CCParams<CryptoContextCKKSRNS> params;
    params.SetMultiplicativeDepth(5);
    params.SetScalingModSize(50);
    params.SetBatchSize(256);
    params.SetFirstModSize(60);

    CryptoContext<DCRTPoly> cc = GenCryptoContext(params);
    cc->Enable(PKE);
    cc->Enable(KEYSWITCH);
    cc->Enable(LEVELEDSHE);

    auto keys = cc->KeyGen();
    cc->EvalMultKeyGen(keys.secretKey);
    auto slots = cc->GetEncodingParams()->GetBatchSize();

    auto make_ct = [&](double val) {
        std::vector<std::complex<double>> vec(slots, {0.0, 0.0});
        vec[0] = {val, 0.0};
        return cc->Encrypt(keys.publicKey, cc->MakeCKKSPackedPlaintext(vec));
    };

    auto decrypt_val = [&](auto ct) {
        Plaintext pt;
        cc->Decrypt(keys.secretKey, ct, &pt);
        return pt->GetCKKSPackedValue()[0].real();
    };

    // Period-2 oscillation
    auto period2 = [&](auto x) {
        return cc->EvalSub(make_ct(phi_sq), x);
    };
    
    // Period-3 oscillation
    auto period3 = [&](auto x) {
        auto added = cc->EvalAdd(x, make_ct(phi_sq));
        return cc->EvalSub(make_ct(phi_cu), added);
    };
    
    // COMBINATION: Alternate between period-2 and period-3
    auto combined = [&](auto x, int step) {
        if (step % 2 == 0) {
            return period2(x);
        } else {
            return period3(x);
        }
    };
    
    std::cout << "COMBINED OSCILLATION (20 steps):\n";
    std::cout << "================================\n\n";
    
    auto current = make_ct(0.0);
    std::vector<double> values;
    
    for (int i = 0; i < 20; i++) {
        current = combined(current, i);
        double v = decrypt_val(current);
        values.push_back(v);
        std::cout << "  Step " << i << ": " << v << "\n";
        
        // Check for period
        if (i > 0 && std::abs(v - values[0]) < 0.01) {
            std::cout << "  → Period " << (i+1) << " found!\n";
            break;
        }
    }
    
    std::cout << "\n========================================\n";
    std::cout << "  KEY: Kung period-6 o period-8,\n";
    std::cout << "  kaya nating i-encode ang Rule 110!\n";
    std::cout << "========================================\n";

    return 0;
}
