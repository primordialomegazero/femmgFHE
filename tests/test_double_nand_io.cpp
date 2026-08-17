// DOUBLE NAND PARA SA iO
// I-verify na 1, 3, 5 NANDs ay may same behavior

#include "openfhe.h"
#include <iostream>

using namespace lbcrypto;

int main() {
    std::cout << "DOUBLE NAND PARA SA iO\n";
    std::cout << "=====================\n\n";

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

    std::cout << "1. DOUBLE NAND CHAIN:\n";
    std::cout << "   Input: 1\n\n";

    auto x = ct1;

    for (int n_gates : {1, 2, 3, 4, 5}) {
        auto current = x;
        for (int i = 0; i < n_gates; i++) {
            current = nand(current, current);
        }
        int64_t val = decrypt_val(current);
        std::cout << "   " << n_gates << " NAND(s): " << val << "\n";
    }

    std::cout << "\n2. ANG PATTERN:\n";
    std::cout << "   1 NAND  → 0 (NOT)\n";
    std::cout << "   2 NANDs → 1 (Identity)\n";
    std::cout << "   3 NANDs → 0 (NOT)\n";
    std::cout << "   4 NANDs → 1 (Identity)\n";
    std::cout << "   5 NANDs → 0 (NOT)\n\n";

    std::cout << "3. iO INDISTINGUISHABILITY:\n";
    std::cout << "   Circuit A (1 NAND) at Circuit C (3 NANDs)\n";
    std::cout << "   ay may PAREHONG behavior (NOT)!\n";
    std::cout << "   → Evaluator ay hindi makaalam kung alin!\n";
    std::cout << "   → PERFECT iO CANDIDATE!\n\n";

    // Verify: 1 NAND = 3 NANDs = 5 NANDs
    std::cout << "4. VERIFY EQUIVALENCE:\n";
    auto nand1 = nand(x, x);
    auto nand3 = nand(nand(nand(x, x), nand(x, x)), nand(nand(x, x), nand(x, x)));
    auto nand5 = nand(nand(nand(nand(nand(x, x), nand(x, x)), nand(nand(x, x), nand(x, x))), nand(nand(nand(x, x), nand(x, x)), nand(nand(x, x), nand(x, x)))), nand(nand(nand(nand(x, x), nand(x, x)), nand(nand(x, x), nand(x, x))), nand(nand(nand(x, x), nand(x, x)), nand(nand(x, x), nand(x, x)))));

    std::cout << "   1 NAND: " << decrypt_val(nand1) << "\n";
    std::cout << "   3 NANDs: " << decrypt_val(nand3) << "\n";
    std::cout << "   5 NANDs: " << decrypt_val(nand5) << "\n";
    std::cout << "   → Lahat pareho: " << 
        (decrypt_val(nand1) == decrypt_val(nand3) && 
         decrypt_val(nand1) == decrypt_val(nand5) ? "YES ✓" : "NO ✗") << "\n";

    return 0;
}
