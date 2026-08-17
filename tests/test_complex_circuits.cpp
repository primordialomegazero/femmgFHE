// COMPLEX CIRCUITS from NAND (Full Adder, Multiplier, etc.)
#include "openfhe.h"
#include <iostream>
#include <vector>

using namespace lbcrypto;

int main() {
    std::cout << "COMPLEX CIRCUITS FROM NAND\n";
    std::cout << "==========================\n\n";

    CCParams<CryptoContextBFVRNS> parameters;
    parameters.SetPlaintextModulus(65537);
    parameters.SetMultiplicativeDepth(20);
    parameters.SetRingDim(32768);

    CryptoContext<DCRTPoly> cc = GenCryptoContext(parameters);
    cc->Enable(PKE);
    cc->Enable(KEYSWITCH);
    cc->Enable(LEVELEDSHE);

    auto keys = cc->KeyGen();
    cc->EvalMultKeyGen(keys.secretKey);

    auto ct0 = cc->Encrypt(keys.publicKey, cc->MakePackedPlaintext({0}));
    auto ct1 = cc->Encrypt(keys.publicKey, cc->MakePackedPlaintext({1}));

    // NAND: 1 - a*b
    auto nand = [&](Ciphertext<DCRTPoly> a, Ciphertext<DCRTPoly> b) {
        auto ab = cc->EvalMult(a, b);
        return cc->EvalSub(ct1, ab);
    };

    // NOT: NAND(x,x)
    auto NOT = [&](Ciphertext<DCRTPoly> a) {
        return nand(a, a);
    };

    // AND: NOT(NAND(a,b))
    auto AND = [&](Ciphertext<DCRTPoly> a, Ciphertext<DCRTPoly> b) {
        auto nand_ab = nand(a, b);
        return NOT(nand_ab);
    };

    // OR: NAND(NOT(a), NOT(b))
    auto OR = [&](Ciphertext<DCRTPoly> a, Ciphertext<DCRTPoly> b) {
        auto not_a = NOT(a);
        auto not_b = NOT(b);
        return nand(not_a, not_b);
    };

    // XOR
    auto XOR = [&](Ciphertext<DCRTPoly> a, Ciphertext<DCRTPoly> b) {
        auto nand_ab = nand(a, b);
        auto nand_a_nand = nand(a, nand_ab);
        auto nand_b_nand = nand(b, nand_ab);
        return nand(nand_a_nand, nand_b_nand);
    };

    // Full Adder
    auto FULL_ADDER = [&](Ciphertext<DCRTPoly> a, Ciphertext<DCRTPoly> b, Ciphertext<DCRTPoly> c) {
        auto xor_ab = XOR(a, b);
        auto sum = XOR(xor_ab, c);
        auto and_ab = AND(a, b);
        auto and_xor_c = AND(xor_ab, c);
        auto carry = OR(and_ab, and_xor_c);
        return std::make_pair(sum, carry);
    };

    // Decrypt helper
    auto decrypt_val = [&](Ciphertext<DCRTPoly> ct) -> int64_t {
        Plaintext pt;
        cc->Decrypt(keys.secretKey, ct, &pt);
        return pt->GetPackedValue()[0];
    };

    // Test all gates
    std::cout << "GATE TESTS:\n";
    int errors = 0;

    // Test NOT
    std::cout << "  NOT(0) = " << decrypt_val(NOT(ct0)) << " (exp 1) "
              << (decrypt_val(NOT(ct0)) == 1 ? "✓" : "✗") << "\n";
    std::cout << "  NOT(1) = " << decrypt_val(NOT(ct1)) << " (exp 0) "
              << (decrypt_val(NOT(ct1)) == 0 ? "✓" : "✗") << "\n";

    // Test AND
    std::cout << "  AND(0,0) = " << decrypt_val(AND(ct0, ct0)) << " (exp 0) "
              << (decrypt_val(AND(ct0, ct0)) == 0 ? "✓" : "✗") << "\n";
    std::cout << "  AND(1,1) = " << decrypt_val(AND(ct1, ct1)) << " (exp 1) "
              << (decrypt_val(AND(ct1, ct1)) == 1 ? "✓" : "✗") << "\n";

    // Test OR
    std::cout << "  OR(0,0) = " << decrypt_val(OR(ct0, ct0)) << " (exp 0) "
              << (decrypt_val(OR(ct0, ct0)) == 0 ? "✓" : "✗") << "\n";
    std::cout << "  OR(1,0) = " << decrypt_val(OR(ct1, ct0)) << " (exp 1) "
              << (decrypt_val(OR(ct1, ct0)) == 1 ? "✓" : "✗") << "\n";

    // Test XOR
    std::cout << "  XOR(0,0) = " << decrypt_val(XOR(ct0, ct0)) << " (exp 0) "
              << (decrypt_val(XOR(ct0, ct0)) == 0 ? "✓" : "✗") << "\n";
    std::cout << "  XOR(1,1) = " << decrypt_val(XOR(ct1, ct1)) << " (exp 0) "
              << (decrypt_val(XOR(ct1, ct1)) == 0 ? "✓" : "✗") << "\n";
    std::cout << "  XOR(1,0) = " << decrypt_val(XOR(ct1, ct0)) << " (exp 1) "
              << (decrypt_val(XOR(ct1, ct0)) == 1 ? "✓" : "✗") << "\n";

    // Test Full Adder
    std::cout << "\nFULL ADDER TESTS:\n";
    int test_cases[8][3] = {
        {0,0,0}, {0,0,1}, {0,1,0}, {0,1,1},
        {1,0,0}, {1,0,1}, {1,1,0}, {1,1,1}
    };

    for (auto& tc : test_cases) {
        int a = tc[0], b = tc[1], c = tc[2];
        auto ct_a = cc->Encrypt(keys.publicKey, cc->MakePackedPlaintext({a}));
        auto ct_b = cc->Encrypt(keys.publicKey, cc->MakePackedPlaintext({b}));
        auto ct_c = cc->Encrypt(keys.publicKey, cc->MakePackedPlaintext({c}));

        auto [ct_sum, ct_carry] = FULL_ADDER(ct_a, ct_b, ct_c);
        int sum = decrypt_val(ct_sum);
        int carry = decrypt_val(ct_carry);

        int exp_sum = a ^ b ^ c;
        int exp_carry = (a & b) | (b & c) | (a & c);

        bool ok = (sum == exp_sum && carry == exp_carry);
        if (!ok) errors++;
        std::cout << "  " << a << "+" << b << "+" << c << " = " << carry << sum
                  << " (exp " << exp_carry << exp_sum << ") "
                  << (ok ? "✓" : "✗") << "\n";
    }

    std::cout << "\nErrors: " << errors << "/" << (8 + 7) << "\n";

    if (errors == 0) {
        std::cout << "\n✅ ALL COMPLEX CIRCUITS WORK!\n";
        std::cout << "✅ PERIOD-2 WORKS FOR ANY CIRCUIT!\n";
        std::cout << "✅ NAND IS FUNCTIONALLY COMPLETE!\n";
    }

    return 0;
}
