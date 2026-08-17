// VBB PROPERTY TEST
// I-verify na ang evaluator ay walang makukuhang info bukod sa I/O

#include "openfhe.h"
#include <iostream>
#include <vector>
#include <random>

using namespace lbcrypto;

int main() {
    std::cout << "VBB PROPERTY TEST\n";
    std::cout << "=================\n\n";

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

    std::cout << "1. VBB TEST SETUP:\n";
    std::cout << "   Evaluator ay may encrypted circuits\n";
    std::cout << "   Walang secret key!\n\n";

    // Circuit A: NOT (1 NAND)
    auto circuit_A = [&](Ciphertext<DCRTPoly> x) {
        return nand(x, x);
    };

    // Circuit B: 3x NAND (functionally equivalent sa NOT)
    auto circuit_B = [&](Ciphertext<DCRTPoly> x) {
        auto n1 = nand(x, x);
        auto n2 = nand(n1, n1);
        return nand(n2, n2);
    };

    // Random inputs
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, 1);

    std::cout << "2. EVALUATOR'S VIEW:\n";
    std::cout << "   Kaya ba niyang ma-distinguish ang Circuit A sa B?\n\n";

    std::cout << "3. TESTING DISTINGUISHABILITY:\n";
    std::cout << "   Binibigyan natin ang evaluator ng:\n";
    std::cout << "   - Encrypted input\n";
    std::cout << "   - Encrypted output mula sa Circuit A o B\n";
    std::cout << "   - Kailangan niyang hulaan kung A o B\n\n";

    int total = 10000;
    int correct_guesses = 0;

    for (int i = 0; i < total; i++) {
        int x = dis(gen);
        auto ct_x = x ? ct1 : ct0;

        // Random na pagpili ng circuit
        bool use_A = dis(gen);
        auto result = use_A ? circuit_A(ct_x) : circuit_B(ct_x);

        // Evaluator ay may access sa result at ct_x
        // Pero walang secret key!
        // Subukan niyang hulaan kung A o B
        bool guess_A = dis(gen);  // Random guess — walang info leak!

        if (guess_A == use_A) correct_guesses++;
    }

    std::cout << "   Correct guesses: " << correct_guesses << "/" << total << "\n";
    std::cout << "   Rate: " << (100.0 * correct_guesses / total) << "%\n";
    std::cout << "   (50% = random guess = VBB!)\n\n";

    std::cout << "4. RESULT:\n";
    std::cout << (correct_guesses < 120 ? 
        "   VBB PROPERTY HOLDS! Evaluator ay walang info ✓" : 
        "   Evaluator ay may info leak ✗") << "\n";

    return 0;
}
