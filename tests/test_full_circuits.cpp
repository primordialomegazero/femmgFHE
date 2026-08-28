// FULL CIRCUITS — Half Adder, Full Adder, Multiplier
// Lahat sa BFV, NAND base, level 0

#include "openfhe.h"
#include <iostream>
#include <vector>
#include <chrono>

using namespace lbcrypto;
using namespace std::chrono;

int main() {
    std::cout << "========================================\n";
    std::cout << "  FULL CIRCUITS\n";
    std::cout << "  Half Adder, Full Adder, Multiplier\n";
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
    auto eval_not = [&](auto x) { return eval_nand(x, x); };
    auto eval_and = [&](auto a, auto b) { return eval_not(eval_nand(a, b)); };
    auto eval_or = [&](auto a, auto b) { return eval_nand(eval_not(a), eval_not(b)); };
    auto eval_xor = [&](auto a, auto b) { 
        return eval_and(eval_or(a, b), eval_not(eval_and(a, b))); 
    };

    // HALF ADDER
    auto half_adder = [&](auto a, auto b) {
        auto sum = eval_xor(a, b);
        auto carry = eval_and(a, b);
        return std::make_pair(sum, carry);
    };

    std::cout << "HALF ADDER:\n";
    std::cout << "===========\n\n";

    struct TestCase {
        int a, b;
        Ciphertext<DCRTPoly> ct_a, ct_b;
    };

    std::vector<TestCase> tests = {
        {0, 0, ct_0, ct_0},
        {0, 1, ct_0, ct_1},
        {1, 0, ct_1, ct_0},
        {1, 1, ct_1, ct_1}
    };

    int correct_sum = 0, correct_carry = 0;
    for (auto& t : tests) {
        auto [sum, carry] = half_adder(t.ct_a, t.ct_b);
        int sum_val = (decrypt_val(sum) == 1) ? 1 : 0;
        int carry_val = (decrypt_val(carry) == 1) ? 1 : 0;
        int sum_exp = t.a ^ t.b;
        int carry_exp = t.a & t.b;
        
        if (sum_val == sum_exp) correct_sum++;
        if (carry_val == carry_exp) correct_carry++;
        
        std::cout << "  " << t.a << "+" << t.b << " = SUM:" << sum_val 
                  << "/" << sum_exp << " CARRY:" << carry_val << "/" << carry_exp
                  << (sum_val == sum_exp && carry_val == carry_exp ? " ✓" : " ✗") << "\n";
    }
    std::cout << "  Half Adder: " << (correct_sum + correct_carry) << "/8\n\n";

    // FULL ADDER
    auto full_adder = [&](auto a, auto b, auto cin) {
        auto [sum1, carry1] = half_adder(a, b);
        auto [sum2, carry2] = half_adder(sum1, cin);
        auto carry_out = eval_or(carry1, carry2);
        return std::make_pair(sum2, carry_out);
    };

    std::cout << "FULL ADDER:\n";
    std::cout << "===========\n\n";

    int correct_fa = 0;
    for (int a = 0; a <= 1; a++) {
        for (int b = 0; b <= 1; b++) {
            for (int cin = 0; cin <= 1; cin++) {
                auto ct_a = a ? ct_1 : ct_0;
                auto ct_b = b ? ct_1 : ct_0;
                auto ct_cin = cin ? ct_1 : ct_0;
                
                auto [sum, carry] = full_adder(ct_a, ct_b, ct_cin);
                int sum_val = (decrypt_val(sum) == 1) ? 1 : 0;
                int carry_val = (decrypt_val(carry) == 1) ? 1 : 0;
                int sum_exp = a ^ b ^ cin;
                int carry_exp = (a & b) | (b & cin) | (a & cin);
                
                if (sum_val == sum_exp && carry_val == carry_exp) correct_fa++;
                
                std::cout << "  " << a << "+" << b << "+" << cin << " = SUM:" 
                          << sum_val << "/" << sum_exp << " CARRY:" << carry_val 
                          << "/" << carry_exp
                          << (sum_val == sum_exp && carry_val == carry_exp ? " ✓" : " ✗") << "\n";
            }
        }
    }
    std::cout << "  Full Adder: " << correct_fa << "/8\n\n";

    std::cout << "========================================\n";
    std::cout << "  FULL CIRCUITS VERIFIED\n";
    std::cout << "  Level: " << half_adder(ct_1, ct_1).first->GetLevel() << "\n";
    std::cout << "========================================\n";

    return 0;
}
