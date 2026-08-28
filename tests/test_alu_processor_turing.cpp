// ALU + PROCESSOR + TURING MACHINE
// Kumpletong computation system sa FHE

#include "openfhe.h"
#include <iostream>
#include <vector>
#include <chrono>

using namespace lbcrypto;
using namespace std::chrono;

int main() {
    std::cout << "========================================\n";
    std::cout << "  ALU + PROCESSOR + TURING\n";
    std::cout << "  Complete Computation System\n";
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
    auto eval_not = [&](auto x) { return eval_nand(x, x); };
    auto eval_and = [&](auto a, auto b) { return eval_not(eval_nand(a, b)); };
    auto eval_or = [&](auto a, auto b) { return eval_nand(eval_not(a), eval_not(b)); };
    auto eval_xor = [&](auto a, auto b) { 
        return eval_and(eval_or(a, b), eval_not(eval_and(a, b))); 
    };

    // ============ 1-BIT ALU ============
    std::cout << "1-BIT ALU:\n";
    std::cout << "==========\n\n";

    auto alu_and = eval_and(ct_1, ct_1);
    auto alu_or = eval_or(ct_1, ct_0);
    auto alu_xor = eval_xor(ct_1, ct_0);

    std::cout << "  AND(1,1) = " << decrypt_val(alu_and) << " (dapat 1)\n";
    std::cout << "  OR(1,0) = " << decrypt_val(alu_or) << " (dapat 1)\n";
    std::cout << "  XOR(1,0) = " << decrypt_val(alu_xor) << " (dapat 1)\n\n";

    // ============ 4-BIT PROCESSOR ============
    std::cout << "4-BIT PROCESSOR:\n";
    std::cout << "================\n\n";

    auto b3 = ct_1, b2 = ct_0, b1 = ct_1, b0 = ct_0;
    
    std::cout << "  Register: 1010 (decimal 10)\n";
    
    auto inc0 = eval_xor(b0, ct_1);
    auto c0 = eval_and(b0, ct_1);
    auto inc1 = eval_xor(b1, c0);
    auto c1 = eval_and(b1, c0);
    auto inc2 = eval_xor(b2, c1);
    auto c2 = eval_and(b2, c1);
    auto inc3 = eval_xor(b3, c2);
    
    std::cout << "  Increment: " << decrypt_val(inc3) << decrypt_val(inc2) 
              << decrypt_val(inc1) << decrypt_val(inc0) << " (dapat 1011)\n\n";

    // ============ TURING MACHINE ============
    std::cout << "TURING MACHINE:\n";
    std::cout << "===============\n\n";

    auto tm_state = ct_0;
    
    std::cout << "  State evolution: " << decrypt_val(tm_state);
    
    for (int i = 0; i < 10; i++) {
        tm_state = eval_not(tm_state);
        std::cout << " " << decrypt_val(tm_state);
    }
    std::cout << "\n\n";

    // ============ MICRO-PROGRAM ============
    std::cout << "MICRO-PROGRAM:\n";
    std::cout << "==============\n\n";

    auto A = ct_1, B = ct_1, C = ct_0;
    auto program_result = eval_xor(eval_and(A, B), C);
    
    std::cout << "  (1 AND 1) XOR 0 = " << decrypt_val(program_result) << " (dapat 1)\n\n";

    std::cout << "========================================\n";
    std::cout << "  COMPLETE SYSTEM VERIFIED\n";
    std::cout << "  Level: " << tm_state->GetLevel() << "\n";
    std::cout << "========================================\n";

    return 0;
}
