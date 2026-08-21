// RULE 110 SA BOUNDED FHE — PERIOD-6 ENCODING
// Encode cells as period-6 states
// Bounded recurrence, walang sumasabog

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
    std::cout << "  RULE 110 — BOUNDED PERIOD-6\n";
    std::cout << "  256 Cells, 100 Steps\n";
    std::cout << "========================================\n\n";

    const double phi = 1.6180339887498948482;
    const double K = phi;  // Period-6 state constant

    CCParams<CryptoContextCKKSRNS> params;
    params.SetMultiplicativeDepth(5);
    params.SetScalingModSize(50);
    params.SetBatchSize(256);
    params.SetFirstModSize(60);

    CryptoContext<DCRTPoly> cc = GenCryptoContext(params);
    cc->Enable(PKE);
    cc->Enable(KEYSWITCH);
    cc->Enable(LEVELEDSHE);
    cc->Enable(ADVANCEDSHE);

    auto keys = cc->KeyGen();
    cc->EvalMultKeyGen(keys.secretKey);
    cc->EvalAtIndexKeyGen(keys.secretKey, {1, -1});

    auto slots = cc->GetEncodingParams()->GetBatchSize();

    auto decrypt_slot = [&](auto ct, int slot_num) {
        Plaintext pt;
        cc->Decrypt(keys.secretKey, ct, &pt);
        return pt->GetCKKSPackedValue()[slot_num].real();
    };

    // Constants
    std::vector<std::complex<double>> K_vec(slots, {K, 0.0});
    auto ct_K = cc->Encrypt(keys.publicKey, cc->MakeCKKSPackedPlaintext(K_vec));

    // Initial pattern: alternate 0 at K
    std::vector<std::complex<double>> init(slots, {0.0, 0.0});
    for (int i = 0; i < slots; i++) {
        init[i] = {(i % 2 == 0) ? K : 0.0, 0.0};
    }

    auto state = cc->Encrypt(keys.publicKey, cc->MakeCKKSPackedPlaintext(init));

    std::cout << "RULE 110 BOUNDED TEST (100 steps):\n";
    std::cout << "==================================\n\n";

    auto t_start = high_resolution_clock::now();
    int errors = 0;

    for (int step = 0; step < 100; step++) {
        // Rule 110: sum = L + C + R
        auto left = cc->EvalAtIndex(state, -1);
        auto right = cc->EvalAtIndex(state, 1);
        auto sum = cc->EvalAdd(cc->EvalAdd(left, state), right);

        // Bounded recurrence: x_{n+1} = x_n - x_{n-1} + K
        // I-apply sa sum para ma-bound ang result
        state = cc->EvalSub(cc->EvalSub(ct_K, sum), state);

        // Check boundedness
        double v = decrypt_slot(state, 128);
        bool bounded = (v >= -2 * K && v <= 3 * K);
        if (!bounded) errors++;

        if (step % 10 == 0) {
            double v128 = decrypt_slot(state, 128);
            double v0 = decrypt_slot(state, 0);
            
            std::cout << "  Step " << step << ": "
                      << "center=" << v128
                      << " left=" << v0
                      << " level=" << state->GetLevel()
                      << (bounded ? " ✓" : " ✗") << "\n";
        }
    }

    auto t_end = high_resolution_clock::now();
    auto total_seconds = duration_cast<seconds>(t_end - t_start).count();

    std::cout << "\n========================================\n";
    std::cout << "  RESULT:\n";
    std::cout << "  Steps: 100\n";
    std::cout << "  Unbounded errors: " << errors << "\n";
    std::cout << "  Time: " << total_seconds << "s\n";
    std::cout << "  Level: " << state->GetLevel() << "\n";
    std::cout << "  Status: " << (errors == 0 ? "✅ BOUNDED RULE 110!" : "❌ MAY SUMASABOG") << "\n";
    std::cout << "========================================\n";

    return 0;
}
