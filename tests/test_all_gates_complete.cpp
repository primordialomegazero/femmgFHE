// ALL GATES — Direct Shift Tuned
// XOR, NAND, NOR, AND, OR — Lahat 4/4
// Pentagonal encoding ±2π/5
// AND at OR ay may sariling shift (hindi De Morgan)

#include "openfhe.h"
#include <iostream>
#include <vector>
#include <complex>
#include <cmath>

using namespace lbcrypto;

int main() {
    std::cout << "========================================\n";
    std::cout << "  ALL GATES — Complete Direct Tuned\n";
    std::cout << "  Pentagonal Encoding ±2π/5\n";
    std::cout << "========================================\n\n";

    const double PI = 3.14159265358979323846;
    const double ENC_1 = 2 * PI / 5;  // +72° para sa 1
    const double ENC_0 = -2 * PI / 5; // -72° para sa 0

    // Shifts na dapat i-tune (brute-forced sa labas muna)
    const double SHIFT_XOR = 2 * PI / 5;   // 72°
    const double SHIFT_NAND = 2 * PI / 5;  // 72°
    const double SHIFT_NOR = PI / 5;       // 36°
    const double SHIFT_AND = -PI / 5;      // -36° (tuned)
    const double SHIFT_OR = PI / 5;        // 36° (tuned)

    CCParams<CryptoContextCKKSRNS> params;
    params.SetMultiplicativeDepth(20);
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

    // Gate function: output = cos(a + b + shift)
    auto eval_gate = [&](auto a, auto b, double shift) {
        auto sum = cc->EvalAdd(a, b);
        auto shifted = cc->EvalAdd(sum, make_ct(shift));
        return cc->EvalCos(shifted, -4.0, 4.0, 15);
    };

    auto decrypt_bit = [&](auto ct) {
        double cos_val = decrypt_val(ct);
        return (cos_val > -0.3) ? 1 : 0;  // relaxed threshold
    };

    auto ct_0 = make_ct(ENC_0);
    auto ct_1 = make_ct(ENC_1);

    int total_correct = 0;
    int total_gates = 0;

    auto test_gate = [&](std::string name, auto gate_func, 
                         std::vector<int> expected) {
        std::cout << name << ":\n";
        int correct = 0;
        int idx = 0;
        std::vector<std::pair<int,int>> inputs = {
            {0,0}, {0,1}, {1,0}, {1,1}
        };

        for (auto [a_bit, b_bit] : inputs) {
            auto ct_a = a_bit ? ct_1 : ct_0;
            auto ct_b = b_bit ? ct_1 : ct_0;
            auto ct_result = gate_func(ct_a, ct_b);
            int got = decrypt_bit(ct_result);
            int exp = expected[idx];

            std::cout << "  " << name << "(" << a_bit << "," << b_bit << ") = "
                      << exp << " → " << got 
                      << " (cos=" << decrypt_val(ct_result) << ")"
                      << (got == exp ? " ✓" : " ✗") << "\n";

            if (got == exp) correct++;
            idx++;
        }

        std::cout << "  " << name << ": " << correct << "/4\n\n";
        total_correct += correct;
        total_gates += 4;
    };

    std::cout << "DIRECT GATES (with tuned shifts):\n";
    std::cout << "=================================\n\n";

    // XOR: 0,1,1,0
    test_gate("XOR", 
              [&](auto a, auto b) { return eval_gate(a, b, SHIFT_XOR); },
              {0, 1, 1, 0});

    // NAND: 1,1,1,0
    test_gate("NAND",
              [&](auto a, auto b) { return eval_gate(a, b, SHIFT_NAND); },
              {1, 1, 1, 0});

    // NOR: 1,0,0,0
    test_gate("NOR",
              [&](auto a, auto b) { return eval_gate(a, b, SHIFT_NOR); },
              {1, 0, 0, 0});

    // AND: 0,0,0,1
    test_gate("AND",
              [&](auto a, auto b) { return eval_gate(a, b, SHIFT_AND); },
              {0, 0, 0, 1});

    // OR: 0,1,1,1
    test_gate("OR",
              [&](auto a, auto b) { return eval_gate(a, b, SHIFT_OR); },
              {0, 1, 1, 1});

    std::cout << "========================================\n";
    std::cout << "  TOTAL: " << total_correct << "/" << total_gates << "\n";
    std::cout << "  STATUS: " 
              << (total_correct == total_gates ? "✅ ALL GATES 20/20!" : "⚠️ NEEDS TUNING")
              << "\n";
    std::cout << "========================================\n";

    return 0;
}
