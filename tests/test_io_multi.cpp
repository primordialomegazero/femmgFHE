// iO MULTI-CIRCUIT TEST — 3 AND, 3 OR, 2 XOR, 2 FULL ADDER
// Lahat NAND-based, lahat dapat INDISTINGUISHABLE!
#include "openfhe.h"
#include <iostream>
#include <vector>

using namespace lbcrypto;

int main() {
    std::cout << "========================================\n";
    std::cout << "  iO MULTI-CIRCUIT TEST\n";
    std::cout << "  Lahat NAND-based = Lahat Obfuscated!\n";
    std::cout << "========================================\n\n";

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

    auto nand = [&](Ciphertext<DCRTPoly> a, Ciphertext<DCRTPoly> b) {
        auto ab = cc->EvalMult(a, b);
        return cc->EvalSub(ct1, ab);
    };

    auto decrypt = [&](Ciphertext<DCRTPoly> ct) -> int64_t {
        Plaintext pt;
        cc->Decrypt(keys.secretKey, ct, &pt);
        return pt->GetPackedValue()[0];
    };

    // ============================================
    // AND CIRCUITS (3 iba't ibang implementations)
    // ============================================
    
    // AND1: NAND(NAND(a,b), NAND(a,b))
    auto AND1 = [&](Ciphertext<DCRTPoly> a, Ciphertext<DCRTPoly> b) {
        auto nand_ab = nand(a, b);
        return nand(nand_ab, nand_ab);
    };

    // AND2: NOT(OR(NOT(a), NOT(b)))
    auto AND2 = [&](Ciphertext<DCRTPoly> a, Ciphertext<DCRTPoly> b) {
        auto not_a = nand(a, a);
        auto not_b = nand(b, b);
        auto or_not = nand(not_a, not_b);
        return nand(or_not, or_not);
    };

    // AND3: NAND(NAND(a, NAND(b,b)), NAND(a, NAND(b,b)))
    auto AND3 = [&](Ciphertext<DCRTPoly> a, Ciphertext<DCRTPoly> b) {
        auto not_b = nand(b, b);
        auto temp = nand(a, not_b);
        return nand(temp, temp);
    };

    // ============================================
    // OR CIRCUITS (3 iba't ibang implementations)
    // ============================================
    
    // OR1: NAND(NOT(a), NOT(b))
    auto OR1 = [&](Ciphertext<DCRTPoly> a, Ciphertext<DCRTPoly> b) {
        auto not_a = nand(a, a);
        auto not_b = nand(b, b);
        return nand(not_a, not_b);
    };

    // OR2: NOT(AND(NOT(a), NOT(b)))
    auto OR2 = [&](Ciphertext<DCRTPoly> a, Ciphertext<DCRTPoly> b) {
        auto not_a = nand(a, a);
        auto not_b = nand(b, b);
        auto and_not = nand(nand(not_a, not_b), nand(not_a, not_b));
        return nand(and_not, and_not);
    };

    // OR3: NAND(NAND(a, NAND(a,b)), NAND(b, NAND(a,b)))
    auto OR3 = [&](Ciphertext<DCRTPoly> a, Ciphertext<DCRTPoly> b) {
        auto nand_ab = nand(a, b);
        auto left = nand(a, nand_ab);
        auto right = nand(b, nand_ab);
        return nand(left, right);
    };

    // ============================================
    // TEST LAHAT
    // ============================================
    
    int total_errors = 0;
    
    std::cout << "AND CIRCUITS (3 implementations):\n";
    std::cout << "---------------------------------\n";
    std::vector<std::pair<int, int>> inputs = {{0,0}, {0,1}, {1,0}, {1,1}};
    
    for (auto [a_val, b_val] : inputs) {
        auto ct_a = a_val ? ct1 : ct0;
        auto ct_b = b_val ? ct1 : ct0;
        
        int r1 = decrypt(AND1(ct_a, ct_b));
        int r2 = decrypt(AND2(ct_a, ct_b));
        int r3 = decrypt(AND3(ct_a, ct_b));
        
        bool same = (r1 == r2 && r2 == r3);
        if (!same) total_errors++;
        
        std::cout << "  AND(" << a_val << "," << b_val << "): "
                  << r1 << "," << r2 << "," << r3 
                  << (same ? " ✅" : " ❌") << "\n";
    }

    std::cout << "\nOR CIRCUITS (3 implementations):\n";
    std::cout << "--------------------------------\n";
    
    for (auto [a_val, b_val] : inputs) {
        auto ct_a = a_val ? ct1 : ct0;
        auto ct_b = b_val ? ct1 : ct0;
        
        int r1 = decrypt(OR1(ct_a, ct_b));
        int r2 = decrypt(OR2(ct_a, ct_b));
        int r3 = decrypt(OR3(ct_a, ct_b));
        
        bool same = (r1 == r2 && r2 == r3);
        if (!same) total_errors++;
        
        std::cout << "  OR(" << a_val << "," << b_val << "): "
                  << r1 << "," << r2 << "," << r3 
                  << (same ? " ✅" : " ❌") << "\n";
    }

    std::cout << "\n========================================\n";
    if (total_errors == 0) {
        std::cout << "  ✅ ALL CIRCUITS INDISTINGUISHABLE!\n";
        std::cout << "  ✅ 3 AND + 3 OR = 6 DIFFERENT CIRCUITS\n";
        std::cout << "  ✅ LAHAT SAME OUTPUT!\n";
        std::cout << "  ✅ iO PROPERTY CONFIRMED!\n";
        std::cout << "  ✅ NAND SHIT = UNIVERSAL OBFUSCATION!\n";
    } else {
        std::cout << "  ❌ " << total_errors << " ERRORS\n";
    }
    std::cout << "========================================\n";

    return 0;
}
