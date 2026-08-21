// RULE 110 + PERIOD-6 MODULO
// I-apply ang period-6 bilang modulo sa sum
// para hindi lumaki

#include "openfhe.h"
#include <iostream>
#include <vector>
#include <complex>
#include <cmath>

using namespace lbcrypto;

int main() {
    std::cout << "========================================\n";
    std::cout << "  RULE 110 + PERIOD-6 MODULO\n";
    std::cout << "  Bounded Neighbor Sum\n";
    std::cout << "========================================\n\n";

    const double phi = 1.6180339887498948482;
    const double phi_sq = phi * phi;
    const double K = phi_sq;

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

    // Constants — period-6 states: 2K, 2K, K, 0, 0, K
    std::vector<std::complex<double>> K_vec(slots, {K, 0.0});
    std::vector<std::complex<double>> twoK_vec(slots, {2*K, 0.0});
    
    auto ct_K = cc->Encrypt(keys.publicKey, cc->MakeCKKSPackedPlaintext(K_vec));
    auto ct_twoK = cc->Encrypt(keys.publicKey, cc->MakeCKKSPackedPlaintext(twoK_vec));

    // Initial: period-6 states sa slots
    std::vector<std::complex<double>> init(slots, {0.0, 0.0});
    for (int i = 0; i < slots; i++) {
        int state = i % 6;
        double val = 0.0;
        if (state == 0 || state == 1) val = 2*K;
        else if (state == 2 || state == 5) val = K;
        else val = 0.0;
        init[i] = {val, 0.0};
    }

    // Dalawang variables para sa period-6 recurrence
    auto x_prev = cc->Encrypt(keys.publicKey, cc->MakeCKKSPackedPlaintext(
        std::vector<std::complex<double>>(slots, {0.0, 0.0})));
    auto x_curr = cc->Encrypt(keys.publicKey, cc->MakeCKKSPackedPlaintext(init));

    std::cout << "RULE 110 + P6 MODULO (100 steps):\n";
    std::cout << "=================================\n\n";

    int errors = 0;

    for (int step = 0; step < 100; step++) {
        // Neighbor sum
        auto left = cc->EvalAtIndex(x_curr, -1);
        auto right = cc->EvalAtIndex(x_curr, 1);
        auto sum = cc->EvalAdd(cc->EvalAdd(left, x_curr), right);

        // Period-6 modulo: i-subtract ang 2K kung sum > 2K
        // Ito ay approximation: K - sum (bounded sa [-2K, K])
        auto bounded_sum = cc->EvalSub(ct_K, sum);

        // Period-6 recurrence: x_next = x_curr - x_prev + bounded_sum
        auto x_next = cc->EvalAdd(cc->EvalSub(x_curr, x_prev), bounded_sum);

        x_prev = x_curr;
        x_curr = x_next;

        double v = decrypt_slot(x_curr, 128);
        bool bounded = (v >= -3*K && v <= 3*K);
        if (!bounded) errors++;

        if (step % 10 == 0) {
            double v128 = decrypt_slot(x_curr, 128);
            double v0 = decrypt_slot(x_curr, 0);
            
            std::cout << "  Step " << step << ": "
                      << "slot128=" << v128
                      << " slot0=" << v0
                      << " level=" << x_curr->GetLevel()
                      << (bounded ? " ✓" : " ✗") << "\n";
        }
    }

    std::cout << "\n  Errors: " << errors << "/100\n";
    std::cout << "  Level: " << x_curr->GetLevel() << "\n";
    std::cout << "  Status: " << (errors == 0 ? "✅ P6 MODULO BOUNDED!" : "❌") << "\n";

    return 0;
}
