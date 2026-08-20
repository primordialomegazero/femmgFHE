#include "openfhe.h"
#include <iostream>
#include <vector>

using namespace lbcrypto;

int main() {
    std::cout << "========================================\n";
    std::cout << "  MODULUS SWITCHING AS NOISE REDUCTION\n";
    std::cout << "  Bawasan ang noise nang walang decrypt\n";
    std::cout << "========================================\n\n";

    CCParams<CryptoContextBFVRNS> parameters;
    parameters.SetPlaintextModulus(65537);
    parameters.SetMultiplicativeDepth(20);
    parameters.SetRingDim(32768);

    CryptoContext<DCRTPoly> cc = GenCryptoContext(parameters);
    cc->Enable(PKE);
    cc->Enable(KEYSWITCH);
    cc->Enable(LEVELEDSHE);
    cc->Enable(ADVANCEDSHE);

    auto keys = cc->KeyGen();
    cc->EvalMultKeyGen(keys.secretKey);

    std::vector<int64_t> zeros(16384, 0);
    std::vector<int64_t> ones(16384, 1);

    auto ct0 = cc->Encrypt(keys.publicKey, cc->MakePackedPlaintext(zeros));
    auto ct1 = cc->Encrypt(keys.publicKey, cc->MakePackedPlaintext(ones));

    auto decrypt = [&](Ciphertext<DCRTPoly> ct) -> int64_t {
        Plaintext pt;
        cc->Decrypt(keys.secretKey, ct, &pt);
        return pt->GetPackedValue()[0];
    };

    auto nand = [&](Ciphertext<DCRTPoly> a, Ciphertext<DCRTPoly> b) {
        auto ab = cc->EvalMult(a, b);
        return cc->EvalSub(ct1, ab);
    };

    std::cout << "1. NAND CHAIN WITH MODULUS SWITCHING:\n";
    std::cout << "   (Switch modulus every 5 NANDs)\n";
    std::cout << "-----------------------------------\n";

    auto current = ct0;
    int max_correct = 0;
    int first_fail = 0;

    for (int i = 1; i <= 40; i++) {
        current = nand(current, current);
        
        if (i % 5 == 0) {
            current = cc->ModReduce(current);
        }
        
        int expected = (i % 2 == 0) ? 0 : 1;
        int result = decrypt(current);
        if (result == expected) {
            max_correct = i;
        } else {
            first_fail = i;
            break;
        }
    }

    std::cout << "   Max correct: " << max_correct << " NANDs\n";
    if (first_fail > 0) {
        std::cout << "   First fail: " << first_fail << "\n";
    }

    std::cout << "\n2. AGGRESSIVE MODULUS SWITCHING:\n";
    std::cout << "   (Switch every 3 NANDs)\n";
    std::cout << "-----------------------------------\n";

    current = ct0;
    max_correct = 0;
    first_fail = 0;

    for (int i = 1; i <= 40; i++) {
        current = nand(current, current);
        
        if (i % 3 == 0) {
            current = cc->ModReduce(current);
        }
        
        int expected = (i % 2 == 0) ? 0 : 1;
        int result = decrypt(current);
        if (result == expected) {
            max_correct = i;
        } else {
            first_fail = i;
            break;
        }
    }

    std::cout << "   Max correct: " << max_correct << " NANDs\n";
    if (first_fail > 0) {
        std::cout << "   First fail: " << first_fail << "\n";
    }

    std::cout << "\n3. CONSERVATIVE MODULUS SWITCHING:\n";
    std::cout << "   (Switch every 8 NANDs)\n";
    std::cout << "-----------------------------------\n";

    current = ct0;
    max_correct = 0;
    first_fail = 0;

    for (int i = 1; i <= 40; i++) {
        current = nand(current, current);
        
        if (i % 8 == 0) {
            current = cc->ModReduce(current);
        }
        
        int expected = (i % 2 == 0) ? 0 : 1;
        int result = decrypt(current);
        if (result == expected) {
            max_correct = i;
        } else {
            first_fail = i;
            break;
        }
    }

    std::cout << "   Max correct: " << max_correct << " NANDs\n";
    if (first_fail > 0) {
        std::cout << "   First fail: " << first_fail << "\n";
    }

    std::cout << "\n========================================\n";
    std::cout << "  RESULT:\n";
    std::cout << "  - Modulus switching: " 
              << (max_correct > 23 ? "NAKATULONG" : "WALANG EPEKTO") << "\n";
    std::cout << "========================================\n";

    return 0;
}
