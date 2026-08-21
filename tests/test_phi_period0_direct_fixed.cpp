// PERIOD-0 DIRECT — FIXED REPEAT DETECTION
// Exact match check para sa repeats

#include "openfhe.h"
#include <iostream>
#include <vector>
#include <complex>
#include <cmath>
#include <set>

using namespace lbcrypto;

int main() {
    std::cout << "========================================\n";
    std::cout << "  PERIOD-0 DIRECT — FIXED\n";
    std::cout << "  Exact Repeat Detection\n";
    std::cout << "========================================\n\n";

    const double phi_sq = 2.618033988749895;

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

    auto ct_phi_sq = make_ct(phi_sq);
    auto ct_one = make_ct(1.0);

    std::cout << "IRRATIONAL ROTATION (100 steps):\n";
    std::cout << "===============================\n\n";

    auto state = make_ct(0.0);
    std::set<int> seen_values;  // Exact match via rounded values
    int exact_repeats = 0;
    int total_steps = 100;

    for (int step = 0; step < total_steps; step++) {
        state = cc->EvalAdd(state, ct_phi_sq);
        
        double v = decrypt_val(state);
        while (v >= 1.0) {
            state = cc->EvalSub(state, ct_one);
            v = decrypt_val(state);
        }
        while (v < 0.0) {
            state = cc->EvalAdd(state, ct_one);
            v = decrypt_val(state);
        }

        // Exact repeat check: i-round sa 6 decimal places
        int rounded = (int)(v * 1000000);
        if (seen_values.count(rounded)) {
            exact_repeats++;
        }
        seen_values.insert(rounded);

        if (step < 20 || step % 20 == 0) {
            std::cout << "  Step " << step << ": v=" << v 
                      << " level=" << state->GetLevel()
                      << " elements=" << state->GetElements()[0].GetNumOfElements() << "\n";
        }
    }

    std::cout << "\n";
    std::cout << "========================================\n";
    std::cout << "  RESULT:\n";
    std::cout << "  Total steps: " << total_steps << "\n";
    std::cout << "  Unique values: " << seen_values.size() << "\n";
    std::cout << "  Exact repeats: " << exact_repeats << "\n";
    std::cout << "  Level: " << state->GetLevel() << "\n";
    std::cout << "  Elements: " << state->GetElements()[0].GetNumOfElements() << "\n";
    std::cout << "  Status: " << (exact_repeats == 0 && state->GetLevel() == 0 ? 
              "✅ PERIOD-0 CONFIRMED!" : "❌ MAY PROBLEMA") << "\n";
    std::cout << "========================================\n";

    return 0;
}
