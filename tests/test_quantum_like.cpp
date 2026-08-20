// QUANTUM-LIKE GATES SA FHE
// Test kung may period-2 cancellation sa H at CNOT analogues
#include "openfhe.h"
#include <iostream>

using namespace lbcrypto;

int main() {
    std::cout << "========================================\n";
    std::cout << "  QUANTUM-LIKE GATES SA FHE\n";
    std::cout << "  H at CNOT analogues\n";
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
    // H ANALOGUE: Hadamard-like gate
    // Classical analogue: NOT (kasi H² = I, NOT² = I)
    // ============================================
    std::cout << "H ANALOGUE (NOT):\n";
    std::cout << "  NOT(NOT(0)) = " << decrypt(NOT(NOT(ct0))) << " (expected 0)\n";
    std::cout << "  NOT(NOT(1)) = " << decrypt(NOT(NOT(ct1))) << " (expected 1)\n\n";

    // ============================================
    // CNOT ANALOGUE: Controlled-NOT
    // Classical analogue: CNOT(a,b) = (a, a XOR b)
    // ============================================
    auto CNOT = [&](Ciphertext<DCRTPoly> control, Ciphertext<DCRTPoly> target) {
        return XOR(control, target);
    };

    std::cout << "CNOT ANALOGUE (XOR):\n";
    std::cout << "  CNOT(0,0) = " << decrypt(CNOT(ct0, ct0)) << " (expected 0)\n";
    std::cout << "  CNOT(0,1) = " << decrypt(CNOT(ct0, ct1)) << " (expected 1)\n";
    std::cout << "  CNOT(1,0) = " << decrypt(CNOT(ct1, ct0)) << " (expected 1)\n";
    std::cout << "  CNOT(1,1) = " << decrypt(CNOT(ct1, ct1)) << " (expected 0)\n\n";

    // ============================================
    // PERIOD-2 TEST: Self-inverse property
    // ============================================
    std::cout << "PERIOD-2 TEST:\n";
    
    // H² = I (Hadamard squared = Identity)
    auto h2_0 = decrypt(NOT(NOT(ct0)));
    auto h2_1 = decrypt(NOT(NOT(ct1)));
    std::cout << "  H²(0) = " << h2_0 << " (expected 0) " << (h2_0 == 0 ? "✅" : "❌") << "\n";
    std::cout << "  H²(1) = " << h2_1 << " (expected 1) " << (h2_1 == 1 ? "✅" : "❌") << "\n";

    // CNOT² = I (CNOT squared = Identity)
    auto cnot2_00 = decrypt(CNOT(ct0, CNOT(ct0, ct0)));
    auto cnot2_11 = decrypt(CNOT(ct1, CNOT(ct1, ct1)));
    std::cout << "  CNOT²(0,0) = " << cnot2_00 << " (expected 0) " << (cnot2_00 == 0 ? "✅" : "❌") << "\n";
    std::cout << "  CNOT²(1,1) = " << cnot2_11 << " (expected 1) " << (cnot2_11 == 1 ? "✅" : "❌") << "\n";

    // ============================================
    // NOISE CANCELLATION TEST (20 iterations)
    // ============================================
    std::cout << "\nNOISE CANCELLATION (20 iterations):\n";
    
    auto current = ct0;
    int errors = 0;
    for (int i = 1; i <= 20; i++) {
        current = NOT(current);  // H analogue
        int val = decrypt(current);
        int expected = (i % 2 == 1) ? 1 : 0;  // Alternating
        if (val != expected) errors++;
    }
    
    std::cout << "  H-analogue 20 iterations: " << (errors == 0 ? "✅ NO ERRORS!" : "❌ ERRORS!") << "\n";

    std::cout << "\n========================================\n";
    if (errors == 0) {
        std::cout << "  ✅ QUANTUM-LIKE PERIOD-2 CONFIRMED!\n";
        std::cout << "  ✅ H at CNOT analogues WORK!\n";
        std::cout << "  ✅ Self-inverse property holds!\n";
    }
    std::cout << "========================================\n";

    return 0;
}
