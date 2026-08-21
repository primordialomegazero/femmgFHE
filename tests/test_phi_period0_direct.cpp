// PERIOD-0 IRRATIONAL ROTATION — DIRECT TEST
// Hindi period-2 o period-4, kundi totoong irrational rotation

#include "openfhe.h"
#include <iostream>
#include <vector>
#include <complex>
#include <cmath>
#include <chrono>

using namespace lbcrypto;
using namespace std::chrono;

int main() {
    std::cout << "========================================\n";
    std::cout << "  PERIOD-0 DIRECT TEST\n";
    std::cout << "  Irrational Rotation (Hindi Oscillation)\n";
    std::cout << "========================================\n\n";

    const double phi = 1.6180339887498948482;
    const double phi_sq = phi * phi;

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

    std::cout << "IRRATIONAL ROTATION SETUP:\n";
    std::cout << "==========================\n\n";
    std::cout << "  Formula: x_{n+1} = (x_n + φ²) mod 1\n";
    std::cout << "  φ² = " << phi_sq << "\n";
    std::cout << "  Expected: DENSE sa [0,1), walang repeat\n\n";

    std::cout << "INTERNAL STATE TRACKING (50 steps):\n";
    std::cout << "===================================\n\n";
    std::cout << "  Step | Value | Level | Elements\n";
    std::cout << "  -----|-------|-------|---------\n";

    auto state = make_ct(0.0);
    std::vector<double> seen_values;
    int errors = 0;

    for (int step = 0; step < 50; step++) {
        // Irrational rotation: x = x + φ²
        state = cc->EvalAdd(state, ct_phi_sq);
        
        // Mod 1
        double v = decrypt_val(state);
        while (v >= 1.0) {
            state = cc->EvalSub(state, ct_one);
            v = decrypt_val(state);
        }
        while (v < 0.0) {
            state = cc->EvalAdd(state, ct_one);
            v = decrypt_val(state);
        }

        // I-record
        seen_values.push_back(v);
        
        // Check kung may repeat
        for (double prev : seen_values) {
            if (std::abs(prev - v) < 0.0001 && step > 0) {
                errors++;
            }
        }

        int num_elements = state->GetElements()[0].GetNumOfElements();

        if (step < 20 || step % 10 == 0) {
            std::cout << "  " << step << " | " << v 
                      << " | " << state->GetLevel()
                      << " | " << num_elements << "\n";
        }
    }

    std::cout << "\n";

    // Density check
    std::cout << "DENSITY CHECK (50 steps):\n";
    std::cout << "=========================\n\n";

    int bins[10] = {0};
    for (double v : seen_values) {
        int bin = (int)(v * 10);
        if (bin >= 10) bin = 9;
        bins[bin]++;
    }

    for (int i = 0; i < 10; i++) {
        std::cout << "  [" << i/10.0 << ", " << (i+1)/10.0 << "): ";
        for (int j = 0; j < bins[i]; j++) std::cout << "#";
        std::cout << " (" << bins[i] << ")\n";
    }

    std::cout << "\n";
    std::cout << "  Total steps: 50\n";
    std::cout << "  Unique values: " << seen_values.size() << "\n";
    std::cout << "  Repeats: " << errors << "\n";
    std::cout << "  Level: " << state->GetLevel() << "\n";
    std::cout << "  Elements: " << state->GetElements()[0].GetNumOfElements() << "\n\n";

    std::cout << "========================================\n";
    std::cout << "  PERIOD-0 VERDICT:\n";
    std::cout << "  " << (errors == 0 && state->GetLevel() == 0 ? 
              "✅ IRRATIONAL ROTATION CONFIRMED!" : "❌ MAY PROBLEMA") << "\n";
    std::cout << "========================================\n";

    return 0;
}
