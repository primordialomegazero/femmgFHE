// RULE 110 BFV FINAL — Emergent Formula
// next = C + R - C*R - L*C*R
// 8/8 perpekto sa BFV

#include "openfhe.h"
#include <iostream>
#include <vector>

using namespace lbcrypto;

int main() {
    std::cout << "========================================\n";
    std::cout << "  RULE 110 BFV FINAL\n";
    std::cout << "  next = C + R - C*R - L*C*R\n";
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

    // Rule 110: next = C + R - C*R - L*C*R
    auto eval_rule110 = [&](auto L, auto C, auto R) {
        auto CR = cc->EvalMult(C, R);
        auto LCR = cc->EvalMult(L, CR);
        
        auto sum1 = cc->EvalAdd(C, R);
        auto sum2 = cc->EvalSub(sum1, CR);
        return cc->EvalSub(sum2, LCR);
    };

    std::cout << "RULE 110 TRUTH TABLE:\n";
    std::cout << "====================\n\n";

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

    std::cout << "\n  Rule 110: " << correct << "/8\n";
    std::cout << "  Level: " << eval_rule110(ct_1, ct_1, ct_1)->GetLevel() << "\n";

    return 0;
}
