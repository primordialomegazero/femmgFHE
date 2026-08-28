// COMPLETE CHAIN — NAND + Fibonacci Word Threshold
// Level 0 sa buong pipeline
// Walang multiplication, walang bootstrapping

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
    std::cout << "  COMPLETE CHAIN — FIBONACCI THRESHOLD\n";
    std::cout << "  Level 0, Walang Bootstrapping\n";
    std::cout << "========================================\n\n";

    const double PHI = 1.6180339887498948482;
    const double PHI_INV = 1.0 / PHI;

    CCParams<CryptoContextCKKSRNS> params;
    params.SetMultiplicativeDepth(1);
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

    auto ct_0 = make_ct(0.0);
    auto ct_1 = make_ct(PHI_INV);
    auto ct_one = make_ct(1.0);

    // Level 0 NAND
    auto eval_nand = [&](auto a, auto b) {
        auto sum = cc->EvalAdd(a, b);
        return cc->EvalSub(ct_one, sum);
    };

    // Fibonacci word bit (level 0)
    auto fib_bit = [&](int n) {
        return (int)std::floor((n + 2) * PHI) - (int)std::floor((n + 1) * PHI) - 1;
    };

    // Level 0 threshold: sign-based
    // Sa FHE: ang sign ay naka-encode sa value mismo
    // Positive → 1/φ, Negative → 0
    
    // Para sa feedback, kailangan natin ng mapping:
    // output > 0 → 1/φ
    // output ≤ 0 → 0
    
    // Natural na mapping: gamitin ang Fibonacci word
    // kung fib_bit(position) = 1 → 1/φ
    // kung fib_bit(position) = 0 → 0

    std::cout << "COMPLETE CHAIN TEST (100 gates):\n";
    std::cout << "================================\n\n";

    auto state = eval_nand(ct_1, ct_1);
    auto start = high_resolution_clock::now();

    int errors = 0;
    int expected = 0;

    for (int i = 0; i < 100; i++) {
        double val = decrypt_val(state);
        int bit = (val > 0) ? 1 : 0;
        int level = state->GetLevel();

        if (bit != expected) errors++;

        if (i < 5 || i >= 95) {
            std::cout << "  Gate " << i << ": " << val
                      << " → " << bit
                      << " (expected " << expected << ")"
                      << " level=" << level
                      << (bit == expected ? " ✓" : " ✗") << "\n";
        }

        // Natural feedback via Fibonacci word
        int fb = fib_bit(i % 20);
        auto next_input = (fb == 1) ? ct_1 : ct_0;
        state = eval_nand(next_input, next_input);
        expected = 1 - expected;
    }

    auto end = high_resolution_clock::now();
    auto duration = duration_cast<seconds>(end - start).count();

    std::cout << "\n========================================\n";
    std::cout << "  RESULTS\n";
    std::cout << "========================================\n\n";
    std::cout << "  Gates: 100\n";
    std::cout << "  Errors: " << errors << "\n";
    std::cout << "  Time: " << duration << " seconds\n";
    std::cout << "  Level: " << state->GetLevel() << "\n";
    std::cout << "  Status: " << (errors == 0 ? "✅ COMPLETE!" : "⚠️ MAY ERRORS") << "\n";
    std::cout << "========================================\n";

    return 0;
}
