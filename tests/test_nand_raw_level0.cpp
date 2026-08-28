// NAND FROM RAW AND + NEGATION — LEVEL 0 CHECK
// Walang EvalSin/Cos, walang multiplication
// Pentagonal ±2π/5 + natural φ thresholds

#include "openfhe.h"
#include <iostream>
#include <vector>
#include <complex>
#include <cmath>

using namespace lbcrypto;

int main() {
    std::cout << "========================================\n";
    std::cout << "  RAW NAND — LEVEL 0\n";
    std::cout << "  Walang EvalSin/Cos, Walang Mult\n";
    std::cout << "========================================\n\n";

    const double PI = 3.14159265358979323846;
    const double ENC_0 = -2 * PI / 5;
    const double ENC_1 = 2 * PI / 5;
    const double GOLDEN_ANGLE = 2 * PI * (1.0 - 1.0 / 1.618033988749895);
    const double AND_THRESHOLD = GOLDEN_ANGLE - PI/10;

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

    auto ct_0 = make_ct(ENC_0);
    auto ct_1 = make_ct(ENC_1);

    auto eval_sum = [&](auto a, auto b) {
        return cc->EvalAdd(a, b);
    };

    auto eval_not = [&](auto x) {
        return cc->EvalNegate(x);
    };

    auto eval_nand = [&](auto a, auto b) {
        auto sum = eval_sum(a, b);
        return eval_not(sum);
    };

    auto decrypt_bit = [&](auto ct, double threshold) {
        double val = decrypt_val(ct);
        return (val > threshold) ? 1 : 0;
    };

    std::cout << "RAW NAND TEST (Level 0):\n";
    std::cout << "========================\n\n";

    struct TestCase {
        int a_bit;
        int b_bit;
        Ciphertext<DCRTPoly> ct_a;
        Ciphertext<DCRTPoly> ct_b;
    };

    std::vector<TestCase> tests = {
        {0, 0, ct_0, ct_0},
        {0, 1, ct_0, ct_1},
        {1, 0, ct_1, ct_0},
        {1, 1, ct_1, ct_1}
    };

    int correct = 0;
    Ciphertext<DCRTPoly> last_nand;

    for (auto& t : tests) {
        auto ct_sum = eval_sum(t.ct_a, t.ct_b);
        auto ct_nand = eval_nand(t.ct_a, t.ct_b);

        double sum_val = decrypt_val(ct_sum);
        double nand_val = decrypt_val(ct_nand);

        int and_got = (sum_val > AND_THRESHOLD) ? 1 : 0;
        int nand_got = (nand_val < -AND_THRESHOLD) ? 0 : 1;

        int and_exp = t.a_bit & t.b_bit;
        int nand_exp = !(t.a_bit & t.b_bit);

        if (nand_got == nand_exp) correct++;

        std::cout << "  NAND(" << t.a_bit << "," << t.b_bit << ") = "
                  << nand_exp << " → " << nand_got
                  << "  (sum=" << (sum_val * 180.0 / PI) << "°, "
                  << "neg_sum=" << (nand_val * 180.0 / PI) << "°, "
                  << "level=" << ct_nand->GetLevel() << ")"
                  << (nand_got == nand_exp ? " ✓" : " ✗") << "\n";

        last_nand = ct_nand;
    }

    std::cout << "\n  NAND: " << correct << "/4\n";
    std::cout << "  Level: " << last_nand->GetLevel() << "\n";
    std::cout << "  Status: " << (correct == 4 ? "✅ RAW NAND LEVEL 0!" : "⚠️ NEEDS FIX") << "\n";

    return 0;
}
