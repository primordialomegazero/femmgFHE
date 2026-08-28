// RULE 110 SA φ-SPACE — Universal Computation
// Rule 110: next = (left AND NOT(center) AND NOT(right)) OR ...
// Sa φ-space: NOT = φ-x, AND = composed, OR = composed

#include "openfhe.h"
#include <iostream>
#include <vector>
#include <chrono>

using namespace lbcrypto;
using namespace std::chrono;

int main() {
    std::cout << "========================================\n";
    std::cout << "  RULE 110 SA φ-SPACE\n";
    std::cout << "  Universal Computation\n";
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

    // Base gates sa φ-space
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

    // Rule 110: next = (left AND NOT(center) AND NOT(right)) OR ...
    // Next = (left AND NOT center) OR (NOT left AND center AND right)
    auto eval_rule110 = [&](auto left, auto center, auto right) {
        // Term 1: left AND NOT(center)
        auto not_center = eval_not(center);
        auto term1 = eval_and(left, not_center);
        
        // Term 2: NOT(left) AND center AND right
        auto not_left = eval_not(left);
        auto center_and_right = eval_and(center, right);
        auto term2 = eval_and(not_left, center_and_right);
        
        // Next = term1 OR term2
        return eval_or(term1, term2);
    };

    std::cout << "RULE 110 TRUTH TABLE:\n";
    std::cout << "====================\n\n";

    // Test lahat ng combinations ng (left, center, right)
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
        
        // Rule 110 truth table
        int expected;
        if (t.l == 0 && t.c == 0 && t.r == 0) expected = 0;
        else if (t.l == 0 && t.c == 0 && t.r == 1) expected = 1;
        else if (t.l == 0 && t.c == 1 && t.r == 0) expected = 1;
        else if (t.l == 0 && t.c == 1 && t.r == 1) expected = 1;
        else if (t.l == 1 && t.c == 0 && t.r == 0) expected = 0;
        else if (t.l == 1 && t.c == 0 && t.r == 1) expected = 1;
        else if (t.l == 1 && t.c == 1 && t.r == 0) expected = 1;
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
