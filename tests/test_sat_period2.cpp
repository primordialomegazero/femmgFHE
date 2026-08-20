// SAT SOLVING WITH PERIOD-2 NAND
// Test: Kaya ba ng period-2 na mag-solve ng SAT?
#include "openfhe.h"
#include <iostream>
#include <vector>

using namespace lbcrypto;

int main() {
    std::cout << "========================================\n";
    std::cout << "  SAT + PERIOD-2 NAND TEST\n";
    std::cout << "  (3-variable SAT problem)\n";
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

    // ============================================
    // SAT PROBLEM: (x1 OR x2) AND (NOT x1 OR x3) AND (NOT x2 OR NOT x3)
    // ============================================
    std::cout << "SAT FORMULA:\n";
    std::cout << "  (x1 OR x2) AND (NOT x1 OR x3) AND (NOT x2 OR NOT x3)\n\n";

    // Brute force: Subukan lahat ng 2^3 = 8 assignments
    std::cout << "BRUTE FORCE SAT SOLVING:\n";
    std::cout << "------------------------\n";
    
    int sat_count = 0;
    std::vector<int> solutions;
    
    for (int x1 = 0; x1 <= 1; x1++) {
        for (int x2 = 0; x2 <= 1; x2++) {
            for (int x3 = 0; x3 <= 1; x3++) {
                auto ct_x1 = x1 ? ct1 : ct0;
                auto ct_x2 = x2 ? ct1 : ct0;
                auto ct_x3 = x3 ? ct1 : ct0;
                
                // Clause 1: (x1 OR x2)
                auto c1 = OR(ct_x1, ct_x2);
                
                // Clause 2: (NOT x1 OR x3)
                auto c2 = OR(NOT(ct_x1), ct_x3);
                
                // Clause 3: (NOT x2 OR NOT x3)
                auto c3 = OR(NOT(ct_x2), NOT(ct_x3));
                
                // Full formula: c1 AND c2 AND c3
                auto full = AND(AND(c1, c2), c3);
                
                int result = decrypt(full);
                
                std::cout << "  x1=" << x1 << " x2=" << x2 << " x3=" << x3 
                          << " → SAT=" << result << "\n";
                
                if (result == 1) {
                    sat_count++;
                    solutions.push_back((x1 << 2) | (x2 << 1) | x3);
                }
            }
        }
    }

    std::cout << "\n========================================\n";
    std::cout << "  SAT SOLUTIONS: " << sat_count << "\n";
    for (int sol : solutions) {
        std::cout << "  x1=" << ((sol >> 2) & 1) 
                  << " x2=" << ((sol >> 1) & 1) 
                  << " x3=" << (sol & 1) << "\n";
    }
    std::cout << "========================================\n";

    return 0;
}
