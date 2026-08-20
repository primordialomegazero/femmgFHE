// UNSAT TEST — Formula na WALANG solution
#include "openfhe.h"
#include <iostream>

using namespace lbcrypto;

int main() {
    std::cout << "========================================\n";
    std::cout << "  UNSAT TEST (Walang Solution)\n";
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

    auto NOT = [&](Ciphertext<DCRTPoly> a) { return nand(a, a); };
    auto AND = [&](Ciphertext<DCRTPoly> a, Ciphertext<DCRTPoly> b) { 
        return NOT(nand(a, b)); 
    };
    auto OR = [&](Ciphertext<DCRTPoly> a, Ciphertext<DCRTPoly> b) {
        return nand(NOT(a), NOT(b));
    };

    auto decrypt = [&](Ciphertext<DCRTPoly> ct) -> int64_t {
        Plaintext pt;
        cc->Decrypt(keys.secretKey, ct, &pt);
        return pt->GetPackedValue()[0];
    };

    // UNSAT: (x1) AND (NOT x1) — CONTRADICTION!
    std::cout << "UNSAT FORMULA:\n";
    std::cout << "  (x1) AND (NOT x1)\n";
    std::cout << "  (Contradiction — walang solution!)\n\n";

    int sat_count = 0;
    
    for (int x1 = 0; x1 <= 1; x1++) {
        auto ct_x1 = x1 ? ct1 : ct0;
        
        auto clause1 = ct_x1;         // x1
        auto clause2 = NOT(ct_x1);     // NOT x1
        auto formula = AND(clause1, clause2);  // x1 AND NOT x1
        
        int result = decrypt(formula);
        std::cout << "  x1=" << x1 << " → formula = " << result << "\n";
        if (result == 1) sat_count++;
    }

    std::cout << "\n========================================\n";
    if (sat_count == 0) {
        std::cout << "  ✅ UNSAT CONFIRMED!\n";
        std::cout << "  ✅ Walang assignment na nagpapagana!\n";
        std::cout << "  ✅ Period-2 HOLDS sa UNSAT!\n";
    } else {
        std::cout << "  ❌ May " << sat_count << " solution(s)!\n";
    }
    std::cout << "========================================\n";

    return 0;
}
