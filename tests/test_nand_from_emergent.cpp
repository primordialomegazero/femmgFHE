// NAND FROM EMERGENT AND + NATURAL NOT
// Pentagonal ±2π/5
// AND = sin > 0.5
// NOT = EvalNegate
// NAND = NOT(AND)

#include "openfhe.h"
#include <iostream>
#include <vector>
#include <complex>
#include <cmath>

using namespace lbcrypto;

int main() {
    std::cout << "========================================\n";
    std::cout << "  NAND FROM EMERGENT AND + NOT\n";
    std::cout << "  Pentagonal ±2π/5\n";
    std::cout << "========================================\n\n";

    const double PI = 3.14159265358979323846;
    const double ENC_0 = -2 * PI / 5;
    const double ENC_1 = 2 * PI / 5;

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

    auto ct_0 = make_ct(ENC_0);
    auto ct_1 = make_ct(ENC_1);

    // Emergent gates
    auto eval_and = [&](auto a, auto b) {
        auto sum = cc->EvalAdd(a, b);
        return cc->EvalSin(sum, -4.0, 4.0, 15);  // AND = sin > 0.5
    };

    auto eval_not = [&](auto x) {
        return cc->EvalNegate(x);  // NOT = -x
    };

    auto eval_nand = [&](auto a, auto b) {
        auto and_result = eval_and(a, b);
        return eval_not(and_result);  // NAND = -sin(a+b)
    };

    auto decrypt_bit = [&](auto ct, double threshold) {
        double val = decrypt_val(ct);
        return (val > threshold) ? 1 : 0;
    };

    std::cout << "EMERGENT NAND TEST:\n";
    std::cout << "====================\n\n";

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
    for (auto& t : tests) {
        auto ct_and = eval_and(t.ct_a, t.ct_b);
        auto ct_nand = eval_nand(t.ct_a, t.ct_b);

        double and_val = decrypt_val(ct_and);
        double nand_val = decrypt_val(ct_nand);

        int and_got = (and_val > 0.5) ? 1 : 0;
        int nand_got = (nand_val > -0.5) ? 1 : 0;  // inverted threshold

        int and_exp = t.a_bit & t.b_bit;
        int nand_exp = !(t.a_bit & t.b_bit);

        if (nand_got == nand_exp) correct++;

        std::cout << "  NAND(" << t.a_bit << "," << t.b_bit << ") = "
                  << nand_exp << " → " << nand_got
                  << "  (AND=" << and_got << "/" << and_exp
                  << ", sin=" << and_val
                  << ", -sin=" << nand_val << ")"
                  << (nand_got == nand_exp ? " ✓" : " ✗") << "\n";
    }

    std::cout << "\n  NAND: " << correct << "/4\n";
    std::cout << "  Status: " << (correct == 4 ? "✅ EMERGENT NAND!" : "⚠️ NEEDS FIX") << "\n";

    return 0;
}
