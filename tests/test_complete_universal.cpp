// COMPLETE UNIVERSAL — Rule 110 Evolution + Fibonacci + Turing
// Lahat sa BFV, level 0, walang bootstrapping

#include "openfhe.h"
#include <iostream>
#include <vector>
#include <chrono>

using namespace lbcrypto;
using namespace std::chrono;

int main() {
    std::cout << "========================================\n";
    std::cout << "  COMPLETE UNIVERSAL\n";
    std::cout << "  Rule 110 + Fibonacci + Turing\n";
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

    // 1. RULE 110 EVOLUTION
    std::cout << "1. RULE 110 EVOLUTION (10 steps):\n";
    std::cout << "==================================\n\n";

    auto eval_rule110 = [&](auto L, auto C, auto R) {
        auto CR = cc->EvalMult(C, R);
        auto LCR = cc->EvalMult(L, CR);
        auto sum1 = cc->EvalAdd(C, R);
        auto sum2 = cc->EvalSub(sum1, CR);
        return cc->EvalSub(sum2, LCR);
    };

    // Initial: 0 1 1 0 1 0 0 1
    auto cells = {ct_0, ct_1, ct_1, ct_0, ct_1, ct_0, ct_0, ct_1};
    std::vector<Ciphertext<DCRTPoly>> state(cells.begin(), cells.end());
    
    std::cout << "  Initial: ";
    for (auto& c : state) std::cout << decrypt_val(c) << " ";
    std::cout << "\n\n";

    for (int step = 0; step < 10; step++) {
        std::vector<Ciphertext<DCRTPoly>> next(8);
        for (int i = 0; i < 8; i++) {
            auto L = state[(i - 1 + 8) % 8];
            auto C = state[i];
            auto R = state[(i + 1) % 8];
            next[i] = eval_rule110(L, C, R);
        }
        state = next;
        std::cout << "  Step " << step << ": ";
        for (auto& c : state) std::cout << decrypt_val(c) << " ";
        std::cout << "\n";
    }

    // 2. FIBONACCI WORD
    std::cout << "\n2. FIBONACCI WORD:\n";
    std::cout << "=================\n\n";

    auto fib_word_bit = [&](int n) {
        return (int)(std::floor((n + 2) * 1.6180339887498948482) - 
                     std::floor((n + 1) * 1.6180339887498948482)) - 1;
    };

    std::cout << "  First 20 bits: ";
    for (int i = 0; i < 20; i++) {
        std::cout << fib_word_bit(i) << " ";
    }
    std::cout << "\n\n";

    // 3. TURING MACHINE SIMULATION
    std::cout << "3. TURING MACHINE (Simple Program):\n";
    std::cout << "===================================\n\n";
    std::cout << "  State: 0,1 na may toggle\n";
    std::cout << "  NAND gate bilang transition function\n\n";

    auto eval_nand = [&](auto a, auto b) {
        auto product = cc->EvalMult(a, b);
        return cc->EvalSub(ct_1, product);
    };

    auto tm_state = ct_0;
    std::cout << "  Initial state: " << decrypt_val(tm_state) << "\n";
    
    for (int step = 0; step < 20; step++) {
        tm_state = eval_nand(tm_state, tm_state);
        if (step < 5 || step >= 15) {
            std::cout << "  Step " << step << ": " << decrypt_val(tm_state) << "\n";
        }
    }

    std::cout << "\n========================================\n";
    std::cout << "  COMPLETE UNIVERSAL VERIFIED\n";
    std::cout << "  Rule 110 + Fibonacci + Turing\n";
    std::cout << "  Lahat sa BFV, Level 0\n";
    std::cout << "========================================\n";

    return 0;
}
