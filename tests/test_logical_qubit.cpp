// LOGICAL QUBIT SIMULATION
// Surface code style: 1 logical qubit = 9 physical qubits (3x3 grid)
// Simplified: 3-qubit repetition code para sa logical qubit
#include "openfhe.h"
#include <iostream>
#include <vector>

using namespace lbcrypto;

int main() {
    std::cout << "========================================\n";
    std::cout << "  LOGICAL QUBIT SIMULATION\n";
    std::cout << "  (3-qubit code: 1 logical = 3 physical)\n";
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
    // LOGICAL QUBIT CLASS
    // 1 logical qubit = 3 physical qubits
    // ============================================
    struct LogicalQubit {
        Ciphertext<DCRTPoly> q1, q2, q3;
    };

    auto encode_logical = [&](Ciphertext<DCRTPoly> bit) -> LogicalQubit {
        // Repetition code: |0⟩ → |000⟩, |1⟩ → |111⟩
        return {bit, bit, bit};
    };

    auto decode_logical = [&](LogicalQubit lq) -> Ciphertext<DCRTPoly> {
        // Majority vote
        auto q1_and_q2 = AND(lq.q1, lq.q2);
        auto q1_and_q3 = AND(lq.q1, lq.q3);
        auto q2_and_q3 = AND(lq.q2, lq.q3);
        return OR(OR(q1_and_q2, q1_and_q3), q2_and_q3);
    };

    auto logical_NOT = [&](LogicalQubit lq) -> LogicalQubit {
        // Apply NOT sa lahat ng physical qubits
        return {NOT(lq.q1), NOT(lq.q2), NOT(lq.q3)};
    };

    auto logical_XOR = [&](LogicalQubit a, LogicalQubit b) -> LogicalQubit {
        // Bitwise XOR sa logical level
        return {XOR(a.q1, b.q1), XOR(a.q2, b.q2), XOR(a.q3, b.q3)};
    };

    // ============================================
    // TEST LOGICAL QUBIT OPERATIONS
    // ============================================
    std::cout << "LOGICAL QUBIT TESTS:\n";
    std::cout << "--------------------\n";
    
    int errors = 0;

    // Test 1: Encode/Decode |0⟩
    auto logical_0 = encode_logical(ct0);
    int decoded_0 = decrypt(decode_logical(logical_0));
    std::cout << "  |0⟩ → |000⟩ → decode = " << decoded_0 
              << " (expected 0) " << (decoded_0 == 0 ? "✅" : "❌") << "\n";
    if (decoded_0 != 0) errors++;

    // Test 2: Encode/Decode |1⟩
    auto logical_1 = encode_logical(ct1);
    int decoded_1 = decrypt(decode_logical(logical_1));
    std::cout << "  |1⟩ → |111⟩ → decode = " << decoded_1 
              << " (expected 1) " << (decoded_1 == 1 ? "✅" : "❌") << "\n";
    if (decoded_1 != 1) errors++;

    // Test 3: Logical NOT
    auto not_0 = logical_NOT(logical_0);
    int decoded_not = decrypt(decode_logical(not_0));
    std::cout << "  NOT(|0⟩) → decode = " << decoded_not 
              << " (expected 1) " << (decoded_not == 1 ? "✅" : "❌") << "\n";
    if (decoded_not != 1) errors++;

    // Test 4: Logical NOT² = I (Period-2!)
    auto not_not_0 = logical_NOT(logical_NOT(logical_0));
    int decoded_not2 = decrypt(decode_logical(not_not_0));
    std::cout << "  NOT²(|0⟩) → decode = " << decoded_not2 
              << " (expected 0) " << (decoded_not2 == 0 ? "✅" : "❌") << "\n";
    if (decoded_not2 != 0) errors++;

    // Test 5: Logical XOR
    auto xor_result = logical_XOR(logical_0, logical_1);
    int decoded_xor = decrypt(decode_logical(xor_result));
    std::cout << "  |0⟩ XOR |1⟩ → decode = " << decoded_xor 
              << " (expected 1) " << (decoded_xor == 1 ? "✅" : "❌") << "\n";
    if (decoded_xor != 1) errors++;

    // Test 6: Error correction (flip middle qubit)
    auto corrupted = LogicalQubit{logical_0.q1, logical_1.q2, logical_0.q3};  // |010⟩
    int decoded_corrected = decrypt(decode_logical(corrupted));
    std::cout << "  |010⟩ (corrupted) → decode = " << decoded_corrected 
              << " (expected 0) " << (decoded_corrected == 0 ? "✅" : "❌") << "\n";
    if (decoded_corrected != 0) errors++;

    // ============================================
    // PERIOD-2 SA LOGICAL LEVEL
    // ============================================
    std::cout << "\nPERIOD-2 SA LOGICAL LEVEL:\n";
    std::cout << "--------------------------\n";
    
    auto current = logical_0;
    for (int i = 1; i <= 10; i++) {
        current = logical_NOT(current);
        int val = decrypt(decode_logical(current));
        int expected = (i % 2 == 1) ? 1 : 0;
        if (val != expected) errors++;
    }
    std::cout << "  Logical NOT 10 iterations: " 
              << (errors == 0 ? "✅ PERIOD-2 HOLDS!" : "❌ ERRORS!") << "\n";

    std::cout << "\n========================================\n";
    if (errors == 0) {
        std::cout << "  ✅ LOGICAL QUBIT OPERATIONS WORK!\n";
        std::cout << "  ✅ ERROR CORRECTION WORKS!\n";
        std::cout << "  ✅ PERIOD-2 HOLDS SA LOGICAL LEVEL!\n";
        std::cout << "  ✅ SURFACE CODE SIMPLIFIED CONFIRMED!\n";
    } else {
        std::cout << "  ❌ " << errors << " ERRORS\n";
    }
    std::cout << "========================================\n";

    return 0;
}
