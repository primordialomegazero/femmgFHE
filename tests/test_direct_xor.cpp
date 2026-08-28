// DIRECT XOR — Walang Composition Error
// XOR(a,b) = (a-b)² — direct formula
// O sa integer space: XOR = |a-b| o (a+b) mod 2

#include "openfhe.h"
#include <iostream>
#include <vector>
#include <complex>
#include <cmath>

using namespace lbcrypto;

int main() {
    std::cout << "========================================\n";
    std::cout << "  DIRECT XOR — Walang Composition Error\n";
    std::cout << "  XOR = (a-b)²\n";
    std::cout << "========================================\n\n";

    CCParams<CryptoContextCKKSRNS> params;
    params.SetMultiplicativeDepth(10);
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

    auto ct_zero = make_ct(0.0);
    auto ct_one = make_ct(1.0);

    // Direct XOR: XOR = (a-b)²
    auto eval_xor_direct = [&](auto a, auto b) {
        auto diff = cc->EvalSub(a, b);
        return cc->EvalMult(diff, diff);
    };

    std::cout << "DIRECT XOR TEST:\n";
    std::cout << "================\n\n";

    struct TestCase {
        int a_bit;
        int b_bit;
        Ciphertext<DCRTPoly> ct_a;
        Ciphertext<DCRTPoly> ct_b;
    };

    std::vector<TestCase> tests = {
        {0, 0, ct_zero, ct_zero},
        {0, 1, ct_zero, ct_one},
        {1, 0, ct_one, ct_zero},
        {1, 1, ct_one, ct_one}
    };

    int correct = 0;
    for (auto& t : tests) {
        auto xor_result = eval_xor_direct(t.ct_a, t.ct_b);
        double val = decrypt_val(xor_result);
        int got = (val > 0.5) ? 1 : 0;
        int expected = t.a_bit ^ t.b_bit;

        if (got == expected) correct++;

        std::cout << "  XOR(" << t.a_bit << "," << t.b_bit << ") = "
                  << expected << " → " << got
                  << " (value=" << val
                  << ", level=" << xor_result->GetLevel() << ")"
                  << (got == expected ? " ✓" : " ✗") << "\n";
    }

    std::cout << "\n  XOR: " << correct << "/4\n";
    std::cout << "  Level: " << eval_xor_direct(ct_one, ct_one)->GetLevel() << "\n";

    return 0;
}
