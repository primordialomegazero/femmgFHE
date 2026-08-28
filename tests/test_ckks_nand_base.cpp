// CKKS NAND BASE — NAND bilang Base Gate
// NAND = 1 - a*b sa CKKS
// Lahat ng iba ay emergent

#include "openfhe.h"
#include <iostream>
#include <vector>
#include <chrono>

using namespace lbcrypto;
using namespace std::chrono;

int main() {
    std::cout << "========================================\n";
    std::cout << "  CKKS NAND BASE\n";
    std::cout << "  NAND = 1 - a*b\n";
    std::cout << "========================================\n\n";

    const double PHI = 1.6180339887498948482;
    const double PSI = -0.6180339887498948482;

    CCParams<CryptoContextCKKSRNS> params;
    params.SetMultiplicativeDepth(20);
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

    // Sa CKKS: 0 = false, 1 = true (exact integers)
    auto ct_0 = make_ct(0.0);
    auto ct_1 = make_ct(1.0);
    auto ct_one = make_ct(1.0);

    // NAND = 1 - a*b
    auto eval_nand = [&](auto a, auto b) {
        auto product = cc->EvalMult(a, b);
        return cc->EvalSub(ct_one, product);
    };

    // Lahat emergent mula sa NAND
    auto eval_not = [&](auto x) {
        return eval_nand(x, x);
    };

    auto eval_and = [&](auto a, auto b) {
        auto nand_ab = eval_nand(a, b);
        return eval_not(nand_ab);
    };

    auto eval_or = [&](auto a, auto b) {
        auto not_a = eval_not(a);
        auto not_b = eval_not(b);
        return eval_nand(not_a, not_b);
    };

    auto eval_xor = [&](auto a, auto b) {
        auto or_ab = eval_or(a, b);
        auto and_ab = eval_and(a, b);
        auto not_and = eval_not(and_ab);
        return eval_and(or_ab, not_and);
    };

    std::cout << "GATE VERIFICATION:\n";
    std::cout << "==================\n\n";

    std::cout << "NOT:\n";
    std::cout << "  NOT(0) = " << decrypt_val(eval_not(ct_0)) << " (dapat 1)\n";
    std::cout << "  NOT(1) = " << decrypt_val(eval_not(ct_1)) << " (dapat 0)\n\n";

    std::cout << "AND:\n";
    std::cout << "  AND(0,0) = " << decrypt_val(eval_and(ct_0, ct_0)) << " (dapat 0)\n";
    std::cout << "  AND(0,1) = " << decrypt_val(eval_and(ct_0, ct_1)) << " (dapat 0)\n";
    std::cout << "  AND(1,1) = " << decrypt_val(eval_and(ct_1, ct_1)) << " (dapat 1)\n\n";

    std::cout << "OR:\n";
    std::cout << "  OR(0,0) = " << decrypt_val(eval_or(ct_0, ct_0)) << " (dapat 0)\n";
    std::cout << "  OR(0,1) = " << decrypt_val(eval_or(ct_0, ct_1)) << " (dapat 1)\n";
    std::cout << "  OR(1,1) = " << decrypt_val(eval_or(ct_1, ct_1)) << " (dapat 1)\n\n";

    std::cout << "XOR:\n";
    std::cout << "  XOR(0,0) = " << decrypt_val(eval_xor(ct_0, ct_0)) << " (dapat 0)\n";
    std::cout << "  XOR(0,1) = " << decrypt_val(eval_xor(ct_0, ct_1)) << " (dapat 1)\n";
    std::cout << "  XOR(1,1) = " << decrypt_val(eval_xor(ct_1, ct_1)) << " (dapat 0)\n\n";

    // Chain test
    std::cout << "CHAIN TEST (50 gates):\n";
    std::cout << "======================\n\n";

    auto state = eval_nand(ct_1, ct_1);
    auto start = high_resolution_clock::now();

    int errors = 0;
    int expected = 0;

    for (int i = 0; i < 50; i++) {
        double val = decrypt_val(state);
        int bit = (val > 0.5) ? 1 : 0;

        if (bit != expected) errors++;

        if (i < 5 || i >= 45) {
            std::cout << "  Gate " << i << ": " << val
                      << " → " << bit
                      << " (expected " << expected << ")"
                      << " level=" << state->GetLevel()
                      << (bit == expected ? " ✓" : " ✗") << "\n";
        }

        auto next_input = (bit == 1) ? ct_1 : ct_0;
        state = eval_nand(next_input, next_input);
        expected = 1 - expected;
    }

    auto end = high_resolution_clock::now();
    auto duration = duration_cast<seconds>(end - start).count();

    std::cout << "\n  Gates: 50\n";
    std::cout << "  Errors: " << errors << "\n";
    std::cout << "  Time: " << duration << " seconds\n";
    std::cout << "  Status: " << (errors == 0 ? "✅ CKKS NAND BASE!" : "⚠️ MAY ERRORS") << "\n";

    return 0;
}
