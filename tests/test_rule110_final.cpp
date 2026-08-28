// RULE 110 FINAL — Mas Malalim na Depth
// AND = a*b/φ, NOT = φ-x, OR = De Morgan

#include "openfhe.h"
#include <iostream>
#include <vector>

using namespace lbcrypto;

int main() {
    std::cout << "========================================\n";
    std::cout << "  RULE 110 FINAL\n";
    std::cout << "  Tamang Gates sa φ-Space\n";
    std::cout << "========================================\n\n";

    const double PHI = 1.6180339887498948482;
    const double PHI_INV = 1.0 / PHI;

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

    auto ct_phi = make_ct(PHI);
    auto ct_0 = make_ct(0.0);
    auto ct_phi_inv = make_ct(PHI_INV);

    auto eval_not = [&](auto x) {
        return cc->EvalSub(ct_phi, x);
    };

    auto eval_and = [&](auto a, auto b) {
        auto mult = cc->EvalMult(a, b);
        return cc->EvalMult(mult, ct_phi_inv);
    };

    auto eval_or = [&](auto a, auto b) {
        auto not_a = eval_not(a);
        auto not_b = eval_not(b);
        auto and_nots = eval_and(not_a, not_b);
        return eval_not(and_nots);
    };

    auto eval_rule110 = [&](auto left, auto center, auto right) {
        // Term 1: NOT left AND center AND NOT right
        auto not_left = eval_not(left);
        auto not_right = eval_not(right);
        auto t1 = eval_and(not_left, center);
        t1 = eval_and(t1, not_right);
        
        // Term 2: NOT left AND center AND right
        auto t2 = eval_and(not_left, center);
        t2 = eval_and(t2, right);
        
        // Term 3: left AND center AND NOT right
        auto t3 = eval_and(left, center);
        t3 = eval_and(t3, not_right);
        
        // Next = t1 OR t2 OR t3
        auto or_12 = eval_or(t1, t2);
        return eval_or(or_12, t3);
    };

    std::cout << "RULE 110 TRUTH TABLE:\n";
    std::cout << "====================\n\n";

    struct TestCase {
        int l, c, r;
        Ciphertext<DCRTPoly> ct_l, ct_c, ct_r;
    };

    std::vector<TestCase> tests = {
        {0, 0, 0, ct_0, ct_0, ct_0},
        {0, 0, 1, ct_0, ct_0, ct_phi},
        {0, 1, 0, ct_0, ct_phi, ct_0},
        {0, 1, 1, ct_0, ct_phi, ct_phi},
        {1, 0, 0, ct_phi, ct_0, ct_0},
        {1, 0, 1, ct_phi, ct_0, ct_phi},
        {1, 1, 0, ct_phi, ct_phi, ct_0},
        {1, 1, 1, ct_phi, ct_phi, ct_phi}
    };

    int correct = 0;
    for (auto& t : tests) {
        auto result = eval_rule110(t.ct_l, t.ct_c, t.ct_r);
        double val = decrypt_val(result);
        int bit = (val > PHI / 2) ? 1 : 0;
        
        int expected;
        switch (t.l * 4 + t.c * 2 + t.r) {
            case 0: expected = 0; break;
            case 1: expected = 1; break;
            case 2: expected = 1; break;
            case 3: expected = 1; break;
            case 4: expected = 0; break;
            case 5: expected = 1; break;
            case 6: expected = 1; break;
            case 7: expected = 0; break;
        }
        
        if (bit == expected) correct++;
        
        std::cout << "  (" << t.l << t.c << t.r << ") → " << bit
                  << " (expected " << expected << ")"
                  << (bit == expected ? " ✓" : " ✗") << "\n";
    }

    std::cout << "\n  Rule 110: " << correct << "/8\n";

    return 0;
}
