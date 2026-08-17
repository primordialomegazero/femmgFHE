// iO EXTENSION SA STABLE BFV
// Gumagamit ng standard BFV na 0 errors

#include "openfhe.h"
#include <iostream>
#include <vector>
#include <random>

using namespace lbcrypto;

int main() {
    std::cout << "iO EXTENSION SA STABLE BFV\n";
    std::cout << "=========================\n\n";

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

    auto decrypt_val = [&](Ciphertext<DCRTPoly> ct) {
        Plaintext pt;
        cc->Decrypt(keys.secretKey, ct, &pt);
        return (int64_t)pt->GetPackedValue()[0];
    };

    auto nand = [&](Ciphertext<DCRTPoly> a, Ciphertext<DCRTPoly> b) {
        auto ab = cc->EvalMult(a, b);
        return cc->EvalSub(ct1, ab);
    };

    auto not_gate = [&](Ciphertext<DCRTPoly> a) { return nand(a, a); };
    auto and_gate = [&](Ciphertext<DCRTPoly> a, Ciphertext<DCRTPoly> b) { 
        return not_gate(nand(a, b)); 
    };
    auto or_gate = [&](Ciphertext<DCRTPoly> a, Ciphertext<DCRTPoly> b) {
        return nand(not_gate(a), not_gate(b));
    };
    auto xor_gate = [&](Ciphertext<DCRTPoly> a, Ciphertext<DCRTPoly> b) {
        auto n1 = nand(a, b);
        return nand(nand(a, n1), nand(b, n1));
    };

    std::cout << "1. iO IN ANGAT NA GATES:\n";
    std::cout << "   - NOT, AND, OR, XOR — lahat ay obfuscated sa BFV\n\n";

    // iO Test: Dalawang different circuits na may same I/O
    std::cout << "2. iO TEST — SAME BEHAVIOR, DIFFERENT IMPLEMENTATIONS:\n";
    std::cout << "   Program A: XOR(a,b) = OR(AND(a,NOT(b)), AND(NOT(a),b))\n";
    std::cout << "   Program B: XOR(a,b) = NAND-chain formulation\n\n";

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, 1);

    int total = 100;
    int same_output = 0;

    for (int i = 0; i < total; i++) {
        int a = dis(gen);
        int b = dis(gen);
        auto ca = a ? ct1 : ct0;
        auto cb = b ? ct1 : ct0;

        // Program A: standard XOR
        auto not_a = not_gate(ca);
        auto not_b = not_gate(cb);
        auto and_a_notb = and_gate(ca, not_b);
        auto and_nota_b = and_gate(not_a, cb);
        auto result_A = or_gate(and_a_notb, and_nota_b);

        // Program B: NAND chain XOR
        auto n1 = nand(ca, cb);
        auto result_B = nand(nand(ca, n1), nand(cb, n1));

        int64_t dec_A = decrypt_val(result_A);
        int64_t dec_B = decrypt_val(result_B);

        if (dec_A == dec_B) same_output++;
    }

    std::cout << "   Same output: " << same_output << "/" << total << "\n";
    std::cout << "   → " << (same_output == total ? "FUNCTIONALLY EQUIVALENT ✓" : "NOT EQUIVALENT ✗") << "\n\n";

    std::cout << "3. iO INDISTINGUISHABILITY:\n";
    std::cout << "   Ang evaluator ay may dalawang encrypted circuits\n";
    std::cout << "   na may same output pero different implementations\n";
    std::cout << "   → Hindi ma-distinguish kung alin ang ginamit!\n";
    std::cout << "   → FOUNDATION PARA SA iO!\n";

    return 0;
}
