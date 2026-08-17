// STANDARD BFV NAND — STABLE
// Gumagamit ng 0/1 encoding (hindi φ-ψ) para hindi sumabog

#include "openfhe.h"
#include <iostream>

using namespace lbcrypto;

int main() {
    std::cout << "STANDARD BFV NAND — STABLE\n";
    std::cout << "==========================\n\n";

    // Standard parameters — 65537 plaintext, 32768 ring
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

    // Standard encoding: 0 at 1
    auto ct0 = cc->Encrypt(keys.publicKey, cc->MakePackedPlaintext({0}));
    auto ct1 = cc->Encrypt(keys.publicKey, cc->MakePackedPlaintext({1}));

    auto decrypt_val = [&](Ciphertext<DCRTPoly> ct) {
        Plaintext pt;
        cc->Decrypt(keys.secretKey, ct, &pt);
        return (int64_t)pt->GetPackedValue()[0];
    };

    // Standard NAND: 1 - a·b
    auto hom_nand = [&](Ciphertext<DCRTPoly> a, Ciphertext<DCRTPoly> b) {
        auto ab = cc->EvalMult(a, b);
        return cc->EvalSub(ct1, ab);
    };

    std::cout << "1. DEEP CHAIN (20 depths):\n";
    auto current = ct1;
    int errors = 0;

    for (int i = 0; i <= 20; i++) {
        int64_t val = decrypt_val(current);
        int64_t expected = (i % 2 == 0) ? 1 : 0;
        bool match = (val == expected);
        if (!match) errors++;

        if (i <= 5 || i % 5 == 0) {
            std::cout << "   Depth " << i << ": " << val << " (exp " << expected << ") "
                      << (match ? "✓" : "✗") << "\n";
        }

        auto sq = cc->EvalMult(current, current);
        current = cc->EvalSub(ct1, sq);
    }

    std::cout << "\n   Errors: " << errors << "/21\n";
    std::cout << "\n2. RESULT: " << (errors == 0 ? "STABLE! ✓" : "May errors") << "\n";

    return 0;
}
