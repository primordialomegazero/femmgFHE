// RULE 110 CORRECT — Tamang Formula
// next = (NOT left AND center) OR (center AND NOT right) OR (left AND NOT center AND right)

#include "openfhe.h"
#include <iostream>
#include <vector>
#include <chrono>

using namespace lbcrypto;
using namespace std::chrono;

int main() {
    std::cout << "========================================\n";
    std::cout << "  RULE 110 CORRECT\n";
    std::cout << "  Tamang Formula\n";
    std::cout << "========================================\n\n";

    const double PHI = 1.6180339887498948482;

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

    auto ct_phi = make_ct(PHI);
    auto ct_0 = make_ct(0.0);

    auto eval_not = [&](auto x) {
        return cc->EvalSub(ct_phi, x);
    };

    auto eval_and = [&](auto a, auto b) {
        auto not_a = eval_not(a);
        auto not_b = eval_not(b);
        auto xor_nots = cc->EvalSub(not_a, not_b);
        return eval_not(xor_nots);
    };

    auto eval_or = [&](auto a, auto b) {
        auto not_a = eval_not(a);
        auto not_b = eval_not(b);
        return eval_not(eval_and(not_a, not_b));
    };

    // Tamang Rule 110:
    // next = (NOT left AND center) OR (center AND NOT right) OR (left AND NOT center AND right)
    auto eval_rule110 = [&](auto left, auto center, auto right) {
        // Term 1: NOT(left) AND center
        auto not_left = eval_not(left);
        auto term1 = eval_and(not_left, center);
        
        // Term 2: center AND NOT(right)
        auto not_right = eval_not(right);
        auto term2 = eval_and(center, not_right);
        
        // Term 3: left AND NOT(center) AND right
        auto not_center = eval_not(center);
        auto left_and_not_center = eval_and(left, not_center);
        auto term3 = eval_and(left_and_not_center, right);
        
        // Next = term1 OR term2 OR term3
        auto or_12 = eval_or(term1, term2);
        return eval_or(or_12, term3);
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
        if (t.l == 1 && t.c == 1 && t.r == 1) expected = 0;
        else if (t.l == 1 && t.c == 1 && t.r == 0) expected = 1;
        else if (t.l == 1 && t.c == 0 && t.r == 1) expected = 1;
        else if (t.l == 1 && t.c == 0 && t.r == 0) expected = 0;
        else if (t.l == 0 && t.c == 1 && t.r == 1) expected = 1;
        else if (t.l == 0 && t.c == 1 && t.r == 0) expected = 1;
        else if (t.l == 0 && t.c == 0 && t.r == 1) expected = 1;
        else expected = 0;
        
        if (bit == expected) correct++;
        
        std::cout << "  (" << t.l << t.c << t.r << ") → " << bit
                  << " (expected " << expected << ")"
                  << (bit == expected ? " ✓" : " ✗") << "\n";
    }

    std::cout << "\n  Rule 110: " << correct << "/8\n";
    std::cout << "  Level: " << eval_rule110(ct_phi, ct_phi, ct_phi)->GetLevel() << "\n";

    return 0;
}
