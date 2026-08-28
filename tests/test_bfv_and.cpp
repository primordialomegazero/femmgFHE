// BFV AND — Natural Multiplication
// Sa BFV, ang AND = a * b ay natural na level 0 (modulo)

#include "openfhe.h"
#include <iostream>
#include <vector>

using namespace lbcrypto;

int main() {
    std::cout << "========================================\n";
    std::cout << "  BFV AND\n";
    std::cout << "  Natural Multiplication\n";
    std::cout << "========================================\n\n";

    CCParams<CryptoContextBFVRNS> params;
    params.SetMultiplicativeDepth(10);
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

    // Sa BFV: 0 = false, 1 = true
    auto ct_0 = make_ct(0);
    auto ct_1 = make_ct(1);

    // AND = a * b (natural sa BFV, level 0 with modulo)
    auto eval_and = [&](auto a, auto b) {
        return cc->EvalMult(a, b);
    };

    // NOT = 1 - x
    auto eval_not = [&](auto x) {
        return cc->EvalSub(make_ct(1), x);
    };

    // NAND = 1 - a*b
    auto eval_nand = [&](auto a, auto b) {
        auto and_ab = eval_and(a, b);
        return eval_not(and_ab);
    };

    std::cout << "AND TEST:\n";
    std::cout << "=========\n";
    std::cout << "  AND(0,0) = " << decrypt_val(eval_and(ct_0, ct_0)) << " (dapat 0)\n";
    std::cout << "  AND(0,1) = " << decrypt_val(eval_and(ct_0, ct_1)) << " (dapat 0)\n";
    std::cout << "  AND(1,1) = " << decrypt_val(eval_and(ct_1, ct_1)) << " (dapat 1)\n";
    std::cout << "  Level: " << eval_and(ct_1, ct_1)->GetLevel() << "\n\n";

    std::cout << "NAND TEST:\n";
    std::cout << "==========\n";
    std::cout << "  NAND(0,0) = " << decrypt_val(eval_nand(ct_0, ct_0)) << " (dapat 1)\n";
    std::cout << "  NAND(0,1) = " << decrypt_val(eval_nand(ct_0, ct_1)) << " (dapat 1)\n";
    std::cout << "  NAND(1,1) = " << decrypt_val(eval_nand(ct_1, ct_1)) << " (dapat 0)\n";
    std::cout << "  Level: " << eval_nand(ct_1, ct_1)->GetLevel() << "\n";

    return 0;
}
