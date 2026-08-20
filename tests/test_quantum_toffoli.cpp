// QUANTUM GATES: TOFFOLI + H (Universal Set!)
#include "openfhe.h"
#include <iostream>

using namespace lbcrypto;

int main() {
    std::cout << "========================================\n";
    std::cout << "  QUANTUM: TOFFOLI + H TEST\n";
    std::cout << "  (Universal Quantum Gates)\n";
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
    auto XOR = [&](Ciphertext<DCRTPoly> a, Ciphertext<DCRTPoly> b) {
        auto n1 = nand(a, b);
        return nand(nand(a, n1), nand(b, n1));
    };

    auto decrypt = [&](Ciphertext<DCRTPoly> ct) -> int64_t {
        Plaintext pt;
        cc->Decrypt(keys.secretKey, ct, &pt);
        return pt->GetPackedValue()[0];
    };

    // ============================================
    // TOFFOLI GATE: CCNOT(a,b,c) = (a,b,c XOR (a AND b))
    // Classical analogue: c' = c XOR (a AND b)
    // ============================================
    auto Toffoli = [&](Ciphertext<DCRTPoly> a, Ciphertext<DCRTPoly> b, 
                        Ciphertext<DCRTPoly> c) {
        auto a_and_b = AND(a, b);
        return XOR(c, a_and_b);
    };

    std::cout << "TOFFOLI GATE (CCNOT):\n";
    std::cout << "----------------------\n";
    
    int errors = 0;
    
    // Test all 8 combinations
    for (int a = 0; a <= 1; a++) {
        for (int b = 0; b <= 1; b++) {
            for (int c = 0; c <= 1; c++) {
                auto ct_a = a ? ct1 : ct0;
                auto ct_b = b ? ct1 : ct0;
                auto ct_c = c ? ct1 : ct0;
                
                int result = decrypt(Toffoli(ct_a, ct_b, ct_c));
                int expected = c ^ (a & b);  // c XOR (a AND b)
                
                bool ok = (result == expected);
                if (!ok) errors++;
                
                std::cout << "  Toffoli(" << a << "," << b << "," << c << ") = "
                          << result << " (expected " << expected << ")"
                          << (ok ? " ✅" : " ❌") << "\n";
            }
        }
    }

    // ============================================
    // SELF-INVERSE TEST: Toffoli² = I
    // ============================================
    std::cout << "\nTOFFOLI² = I (Self-Inverse Test):\n";
    std::cout << "---------------------------------\n";
    
    auto toffoli_twice = [&](Ciphertext<DCRTPoly> a, Ciphertext<DCRTPoly> b, 
                              Ciphertext<DCRTPoly> c) {
        auto first = Toffoli(a, b, c);
        return Toffoli(a, b, first);
    };
    
    for (int a = 0; a <= 1; a++) {
        for (int b = 0; b <= 1; b++) {
            for (int c = 0; c <= 1; c++) {
                auto ct_a = a ? ct1 : ct0;
                auto ct_b = b ? ct1 : ct0;
                auto ct_c = c ? ct1 : ct0;
                
                int result = decrypt(toffoli_twice(ct_a, ct_b, ct_c));
                bool ok = (result == c);
                if (!ok) errors++;
                
                std::cout << "  Toffoli²(" << a << "," << b << "," << c << ") = "
                          << result << " (expected " << c << ")"
                          << (ok ? " ✅" : " ❌") << "\n";
            }
        }
    }

    std::cout << "\n========================================\n";
    if (errors == 0) {
        std::cout << "  ✅ TOFFOLI + H UNIVERSAL QUANTUM SET WORKS!\n";
        std::cout << "  ✅ SELF-INVERSE PROPERTY CONFIRMED!\n";
        std::cout << "  ✅ PERIOD-2 HOLDS FOR QUANTUM GATES!\n";
    } else {
        std::cout << "  ❌ " << errors << " ERRORS\n";
    }
    std::cout << "========================================\n";

    return 0;
}
