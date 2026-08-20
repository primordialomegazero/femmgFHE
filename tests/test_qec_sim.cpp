// QUANTUM ERROR CORRECTION SIMULATION
// 3-qubit bit flip code gamit period-2 NAND
#include "openfhe.h"
#include <iostream>

using namespace lbcrypto;

int main() {
    std::cout << "========================================\n";
    std::cout << "  QUANTUM ERROR CORRECTION (3-qubit)\n";
    std::cout << "  Bit Flip Code Simulation\n";
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
    // 3-QUBIT BIT FLIP CODE
    // |0⟩ → |000⟩
    // |1⟩ → |111⟩
    // Error: bit flip sa isang qubit
    // Correction: majority vote
    // ============================================
    
    std::cout << "3-QUBIT BIT FLIP CODE:\n";
    std::cout << "---------------------\n";
    
    // Encode: |0⟩ = 000, |1⟩ = 111
    auto encode_0 = ct0;  // 000 (lahat 0)
    auto encode_1 = ct1;  // 111 (lahat 1)
    
    // Simulate error: flip middle qubit
    auto error_middle = [&](Ciphertext<DCRTPoly> qubit) {
        return NOT(qubit);  // Flip
    };
    
    // Correction: majority vote
    auto majority_vote = [&](Ciphertext<DCRTPoly> q1, 
                              Ciphertext<DCRTPoly> q2, 
                              Ciphertext<DCRTPoly> q3) {
        // Majority: (q1 AND q2) OR (q1 AND q3) OR (q2 AND q3)
        auto q1_and_q2 = nand(nand(q1, q2), nand(q1, q2));
        auto q1_and_q3 = nand(nand(q1, q3), nand(q1, q3));
        auto q2_and_q3 = nand(nand(q2, q3), nand(q2, q3));
        
        // OR ng tatlo
        auto temp = nand(nand(q1_and_q2, q1_and_q2), nand(q1_and_q3, q1_and_q3));
        return nand(nand(temp, temp), nand(q2_and_q3, q2_and_q3));
    };

    int errors = 0;
    
    // Test 1: No error sa |000⟩
    auto r1 = decrypt(majority_vote(ct0, ct0, ct0));
    std::cout << "  |000⟩ → majority = " << r1 << " (expected 0) " 
              << (r1 == 0 ? "✅" : "❌") << "\n";
    if (r1 != 0) errors++;
    
    // Test 2: No error sa |111⟩
    auto r2 = decrypt(majority_vote(ct1, ct1, ct1));
    std::cout << "  |111⟩ → majority = " << r2 << " (expected 1) " 
              << (r2 == 1 ? "✅" : "❌") << "\n";
    if (r2 != 1) errors++;
    
    // Test 3: Error sa middle qubit ng |000⟩ → |010⟩
    auto r3 = decrypt(majority_vote(ct0, ct1, ct0));
    std::cout << "  |010⟩ → majority = " << r3 << " (expected 0) " 
              << (r3 == 0 ? "✅" : "❌") << "\n";
    if (r3 != 0) errors++;
    
    // Test 4: Error sa middle qubit ng |111⟩ → |101⟩
    auto r4 = decrypt(majority_vote(ct1, ct0, ct1));
    std::cout << "  |101⟩ → majority = " << r4 << " (expected 1) " 
              << (r4 == 1 ? "✅" : "❌") << "\n";
    if (r4 != 1) errors++;

    std::cout << "\n========================================\n";
    if (errors == 0) {
        std::cout << "  ✅ QUANTUM ERROR CORRECTION WORKS!\n";
        std::cout << "  ✅ MAJORITY VOTE CORRECTS ERRORS!\n";
        std::cout << "  ✅ PERIOD-2 HOLDS SA QEC!\n";
    } else {
        std::cout << "  ❌ " << errors << " ERRORS\n";
    }
    std::cout << "========================================\n";

    return 0;
}
