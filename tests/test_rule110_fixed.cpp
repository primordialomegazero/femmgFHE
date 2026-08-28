// RULE 110 FIXED — Tamang Formula
// next = (NOT L AND C) OR (C AND NOT R) OR (L AND NOT C AND R)

#include "openfhe.h"
#include <iostream>
#include <vector>
#include <chrono>

using namespace lbcrypto;
using namespace std::chrono;

int main() {
    std::cout << "========================================\n";
    std::cout << "  RULE 110 FIXED\n";
    std::cout << "  Tamang Formula\n";
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

    // Base gates
    auto eval_nand = [&](auto a, auto b) {
        auto product = cc->EvalMult(a, b);
        return cc->EvalSub(make_ct(1), product);
    };

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

    // TAMANG Rule 110:
    // next = (NOT L AND C AND NOT R) OR 
    //        (NOT L AND C AND R) OR
    //        (L AND C AND NOT R) OR
    //        (L AND NOT C AND R)
    auto eval_rule110 = [&](auto L, auto C, auto R) {
        auto not_L = eval_not(L);
        auto not_C = eval_not(C);
        auto not_R = eval_not(R);
        
        // Term 1: NOT L AND C AND NOT R
        auto t1 = eval_and(not_L, C);
        t1 = eval_and(t1, not_R);
        
        // Term 2: NOT L AND C AND R
        auto t2 = eval_and(not_L, C);
        t2 = eval_and(t2, R);
        
        // Term 3: L AND C AND NOT R
        auto t3 = eval_and(L, C);
        t3 = eval_and(t3, not_R);
        
        // Term 4: L AND NOT C AND R
        auto t4 = eval_and(L, not_C);
        t4 = eval_and(t4, R);
        
        // OR lahat
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
            case 0: expected = 0; break;  // 000
            case 1: expected = 1; break;  // 001
            case 2: expected = 1; break;  // 010
            case 3: expected = 1; break;  // 011
            case 4: expected = 0; break;  // 100
            case 5: expected = 1; break;  // 101
            case 6: expected = 1; break;  // 110
            case 7: expected = 0; break;  // 111
        }
        
        if (bit == expected) correct++;
        
        std::cout << "  (" << t.l << t.c << t.r << ") → " << bit
                  << " (expected " << expected << ")"
                  << (bit == expected ? " ✓" : " ✗") << "\n";
    }

    std::cout << "\n  Rule 110: " << correct << "/8\n";

    return 0;
}
