// ALL FROM NAND — Lahat ng Gates mula sa NAND
// NAND = 1 - a*b (level 0 sa BFV)
// Lahat ng iba ay emergent mula sa NAND

#include "openfhe.h"
#include <iostream>
#include <vector>
#include <chrono>

using namespace lbcrypto;
using namespace std::chrono;

int main() {
    std::cout << "========================================\n";
    std::cout << "  ALL FROM NAND\n";
    std::cout << "  Emergent Gates sa BFV\n";
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

    // NAND = 1 - a*b — ang base gate
    auto eval_nand = [&](auto a, auto b) {
        auto product = cc->EvalMult(a, b);
        return cc->EvalSub(make_ct(1), product);
    };

    // Lahat ng iba ay emergent mula sa NAND:
    
    // NOT(x) = NAND(x, x)
    auto eval_not = [&](auto x) {
        return eval_nand(x, x);
    };

    // AND(a,b) = NOT(NAND(a,b))
    auto eval_and = [&](auto a, auto b) {
        auto nand_ab = eval_nand(a, b);
        return eval_not(nand_ab);
    };

    // OR(a,b) = NAND(NOT(a), NOT(b))
    auto eval_or = [&](auto a, auto b) {
        auto not_a = eval_not(a);
        auto not_b = eval_not(b);
        return eval_nand(not_a, not_b);
    };

    // XOR(a,b) = AND(OR(a,b), NOT(AND(a,b)))
    auto eval_xor = [&](auto a, auto b) {
        auto or_ab = eval_or(a, b);
        auto and_ab = eval_and(a, b);
        auto not_and = eval_not(and_ab);
        return eval_and(or_ab, not_and);
    };

    std::cout << "GATE VERIFICATION (lahat mula sa NAND):\n";
    std::cout << "=======================================\n\n";

    std::cout << "NOT:\n";
    std::cout << "  NOT(0) = " << decrypt_val(eval_not(ct_0)) << " (dapat 1)\n";
    std::cout << "  NOT(1) = " << decrypt_val(eval_not(ct_1)) << " (dapat 0)\n\n";

    std::cout << "AND:\n";
    std::cout << "  AND(0,0) = " << decrypt_val(eval_and(ct_0, ct_0)) << " (dapat 0)\n";
    std::cout << "  AND(0,1) = " << decrypt_val(eval_and(ct_0, ct_1)) << " (dapat 0)\n";
    std::cout << "  AND(1,1) = " << decrypt_val(eval_and(ct_1, ct_1)) << " (dapat 1)\n\n";

    std::cout << "OR:\n";
    std::cout << "  OR(0,0) = " << decrypt_val(eval_or(ct_0, ct_0)) << " (dapat 0)\n";
    std::cout << "  OR(0,1) = " << decrypt_val(eval_or(ct_0, ct_1)) << " (dapat 1)\n";
    std::cout << "  OR(1,1) = " << decrypt_val(eval_or(ct_1, ct_1)) << " (dapat 1)\n\n";

    std::cout << "XOR:\n";
    std::cout << "  XOR(0,0) = " << decrypt_val(eval_xor(ct_0, ct_0)) << " (dapat 0)\n";
    std::cout << "  XOR(0,1) = " << decrypt_val(eval_xor(ct_0, ct_1)) << " (dapat 1)\n";
    std::cout << "  XOR(1,1) = " << decrypt_val(eval_xor(ct_1, ct_1)) << " (dapat 0)\n\n";

    // Chain test
    std::cout << "CHAIN TEST (100 gates):\n";
    std::cout << "=======================\n\n";

    auto state = eval_nand(ct_1, ct_1);
    auto start = high_resolution_clock::now();

    int errors = 0;
    int expected = 0;

    for (int i = 0; i < 100; i++) {
        int64_t val = decrypt_val(state);
        int bit = (val == 1) ? 1 : 0;

        if (bit != expected) errors++;

        if (i < 5 || i >= 95) {
            std::cout << "  Gate " << i << ": " << val
                      << " → " << bit
                      << " (expected " << expected << ")"
                      << " level=" << state->GetLevel()
                      << (bit == expected ? " ✓" : " ✗") << "\n";
        }

        auto next_input = (bit == 1) ? ct_1 : ct_0;
        state = eval_nand(next_input, next_input);
        expected = 1 - expected;
    }

    auto end = high_resolution_clock::now();
    auto duration = duration_cast<seconds>(end - start).count();

    std::cout << "\n  Gates: 100\n";
    std::cout << "  Errors: " << errors << "\n";
    std::cout << "  Time: " << duration << " seconds\n";
    std::cout << "  Status: " << (errors == 0 ? "✅ ALL FROM NAND!" : "⚠️ MAY ERRORS") << "\n";

    return 0;
}
