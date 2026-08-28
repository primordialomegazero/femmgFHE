// NAND 4/4 PERIOD-0 — Tamang Formula
// Hanapin ang tamang NAND na 4/4 sa period-0 space

#include "openfhe.h"
#include <iostream>
#include <vector>
#include <chrono>

using namespace lbcrypto;
using namespace std::chrono;

int main() {
    std::cout << "========================================\n";
    std::cout << "  NAND 4/4 PERIOD-0\n";
    std::cout << "  Tamang Formula Hunt\n";
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
    auto ct_one = make_ct(1);
    auto ct_two = make_ct(2);

    // Test iba't ibang NAND formulas
    std::cout << "FORMULA TESTING:\n";
    std::cout << "================\n\n";

    // Formula 1: NAND = 2 - (a+b)
    auto eval_nand_v1 = [&](auto a, auto b) {
        auto sum = cc->EvalAdd(a, b);
        return cc->EvalSub(ct_two, sum);
    };

    auto v1_00 = decrypt_val(eval_nand_v1(ct_0, ct_0));
    auto v1_01 = decrypt_val(eval_nand_v1(ct_0, ct_1));
    auto v1_11 = decrypt_val(eval_nand_v1(ct_1, ct_1));

    std::cout << "Formula 1: NAND = 2 - (a+b)\n";
    std::cout << "  NAND(0,0) = " << v1_00 << " → " << (v1_00 == 2 ? 1 : 0) << "\n";
    std::cout << "  NAND(0,1) = " << v1_01 << " → " << (v1_01 == 1 ? 1 : 0) << "\n";
    std::cout << "  NAND(1,1) = " << v1_11 << " → " << (v1_11 == 0 ? 0 : 1) << "\n\n";

    // Formula 2: NAND = 1 - a*b (multiplication)
    auto eval_nand_v2 = [&](auto a, auto b) {
        auto product = cc->EvalMult(a, b);
        return cc->EvalSub(ct_one, product);
    };

    auto v2_00 = decrypt_val(eval_nand_v2(ct_0, ct_0));
    auto v2_01 = decrypt_val(eval_nand_v2(ct_0, ct_1));
    auto v2_11 = decrypt_val(eval_nand_v2(ct_1, ct_1));

    std::cout << "Formula 2: NAND = 1 - a*b\n";
    std::cout << "  NAND(0,0) = " << v2_00 << " → " << (v2_00 == 1 ? 1 : 0) << "\n";
    std::cout << "  NAND(0,1) = " << v2_01 << " → " << (v2_01 == 1 ? 1 : 0) << "\n";
    std::cout << "  NAND(1,1) = " << v2_11 << " → " << (v2_11 == 0 ? 0 : 1) << "\n";
    std::cout << "  Level: " << eval_nand_v2(ct_1, ct_1)->GetLevel() << "\n\n";

    // Formula 3: NAND = (1-a) + (1-b) - (1-a)*(1-b)
    // Ito ay De Morgan: NOT(AND) = NOT(a) OR NOT(b)
    auto eval_nand_v3 = [&](auto a, auto b) {
        auto not_a = cc->EvalSub(ct_one, a);
        auto not_b = cc->EvalSub(ct_one, b);
        auto not_a_times_not_b = cc->EvalMult(not_a, not_b);
        auto sum_nots = cc->EvalAdd(not_a, not_b);
        return cc->EvalSub(sum_nots, not_a_times_not_b);
    };

    auto v3_00 = decrypt_val(eval_nand_v3(ct_0, ct_0));
    auto v3_01 = decrypt_val(eval_nand_v3(ct_0, ct_1));
    auto v3_11 = decrypt_val(eval_nand_v3(ct_1, ct_1));

    std::cout << "Formula 3: De Morgan NAND\n";
    std::cout << "  NAND(0,0) = " << v3_00 << " → " << (v3_00 == 1 ? 1 : 0) << "\n";
    std::cout << "  NAND(0,1) = " << v3_01 << " → " << (v3_01 == 1 ? 1 : 0) << "\n";
    std::cout << "  NAND(1,1) = " << v3_11 << " → " << (v3_11 == 0 ? 0 : 1) << "\n";
    std::cout << "  Level: " << eval_nand_v3(ct_1, ct_1)->GetLevel() << "\n\n";

    return 0;
}
