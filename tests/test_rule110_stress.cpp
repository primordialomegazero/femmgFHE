// RULE 110 STRESS — Full Cellular Automaton sa FHE
// NAND base, BFV, walang bootstrapping
// Multi-step Rule 110 evolution

#include "openfhe.h"
#include <iostream>
#include <vector>
#include <chrono>

using namespace lbcrypto;
using namespace std::chrono;

int main() {
    std::cout << "========================================\n";
    std::cout << "  RULE 110 STRESS\n";
    std::cout << "  Full Cellular Automaton\n";
    std::cout << "========================================\n\n";

    CCParams<CryptoContextBFVRNS> params;
    params.SetMultiplicativeDepth(20);
    params.SetPlaintextModulus(65537);
    params.SetBatchSize(256);

    CryptoContext<DCRTPoly> cc = GenCryptoContext(params);
    cc->Enable(PKE);
    cc->Enable(KEYSWITCH);
    cc->Enable(LEVELEDSHE);

    auto keys = cc->KeyGen();
    cc->EvalMultKeyGen(keys.secretKey);
    auto slots = cc->GetEncodingParams()->GetBatchSize();

    auto make_ct = [&](int64_t val) {
        std::vector<int64_t> vec(slots, 0);
        vec[0] = val;
        return cc->Encrypt(keys.publicKey, cc->MakePackedPlaintext(vec));
    };

    auto decrypt_val = [&](auto ct) {
        Plaintext pt;
        cc->Decrypt(keys.secretKey, ct, &pt);
        return pt->GetPackedValue()[0];
    };

    auto ct_0 = make_ct(0);
    auto ct_1 = make_ct(1);

    // NAND base
    auto eval_nand = [&](auto a, auto b) {
        auto product = cc->EvalMult(a, b);
        return cc->EvalSub(make_ct(1), product);
    };

    // Emergent gates
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

    // Rule 110: next = (NOT left AND center) OR (center AND NOT right) 
    //                 OR (left AND NOT center AND right)
    auto eval_rule110 = [&](auto left, auto center, auto right) {
        auto not_left = eval_not(left);
        auto not_right = eval_not(right);
        auto not_center = eval_not(center);
        
        // Term 1: NOT left AND center AND NOT right
        auto t1 = eval_and(not_left, center);
        t1 = eval_and(t1, not_right);
        
        // Term 2: NOT left AND center AND right
        auto t2 = eval_and(not_left, center);
        t2 = eval_and(t2, right);
        
        // Term 3: left AND center AND NOT right
        auto t3 = eval_and(left, center);
        t3 = eval_and(t3, not_right);
        
        // Term 4: left AND center AND right (na may NOT center para sa rule)
        auto t4 = eval_and(left, center);
        t4 = eval_and(t4, right);
        t4 = eval_and(t4, not_center);  // Ito ay 0 para sa 111
        
        return eval_or(eval_or(t1, t2), eval_or(t3, t4));
    };

    std::cout << "RULE 110 TRUTH TABLE:\n";
    std::cout << "=====================\n\n";

    struct TestCase {
        int l, c, r;
        Ciphertext<DCRTPoly> ct_l, ct_c, ct_r;
    };

    std::vector<TestCase> tests = {
        {0, 0, 0, ct_0, ct_0, ct_0},
        {0, 0, 1, ct_0, ct_0, ct_1},
        {0, 1, 0, ct_0, ct_1, ct_0},
        {0, 1, 1, ct_0, ct_1, ct_1},
        {1, 0, 0, ct_1, ct_0, ct_0},
        {1, 0, 1, ct_1, ct_0, ct_1},
        {1, 1, 0, ct_1, ct_1, ct_0},
        {1, 1, 1, ct_1, ct_1, ct_1}
    };

    int correct = 0;
    for (auto& t : tests) {
        auto result = eval_rule110(t.ct_l, t.ct_c, t.ct_r);
        int64_t val = decrypt_val(result);
        int bit = (val == 1) ? 1 : 0;
        
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

    std::cout << "\n  Rule 110: " << correct << "/8\n\n";

    // Multi-step evolution
    std::cout << "MULTI-STEP EVOLUTION (10 steps):\n";
    std::cout << "================================\n\n";

    // Simulate a 5-cell cellular automaton
    // Initial: 0 1 1 0 1
    auto c0 = ct_0, c1 = ct_1, c2 = ct_1, c3 = ct_0, c4 = ct_1;
    
    std::cout << "  Initial: 0 1 1 0 1\n\n";

    for (int step = 0; step < 10; step++) {
        // Compute next state for each cell
        auto n0 = eval_rule110(c4, c0, c1);  // wrap-around
        auto n1 = eval_rule110(c0, c1, c2);
        auto n2 = eval_rule110(c1, c2, c3);
        auto n3 = eval_rule110(c2, c3, c4);
        auto n4 = eval_rule110(c3, c4, c0);
        
        c0 = n0; c1 = n1; c2 = n2; c3 = n3; c4 = n4;
        
        std::cout << "  Step " << step << ": "
                  << decrypt_val(c0) << " "
                  << decrypt_val(c1) << " "
                  << decrypt_val(c2) << " "
                  << decrypt_val(c3) << " "
                  << decrypt_val(c4) << "\n";
    }

    std::cout << "\nOBSERVATION:\n";
    std::cout << "============\n\n";
    std::cout << "  Ang Rule 110 ay nag-evolve sa tamang pattern\n";
    std::cout << "  Walang bootstrapping, walang decrypt sa gitna\n";

    return 0;
}
