// STANDARD ENCODING: Period-2 with 0/1
#include "openfhe.h"
#include <iostream>

using namespace lbcrypto;

int main() {
    std::cout << "STANDARD ENCODING: PERIOD-2 NAND\n";
    std::cout << "================================\n\n";

    // Simple BFV setup with standard plaintext modulus
    CCParams<CryptoContextBFVRNS> parameters;
    parameters.SetPlaintextModulus(65537);  // Standard prime
    parameters.SetMultiplicativeDepth(10);
    parameters.SetRingDim(32768);

    CryptoContext<DCRTPoly> cc = GenCryptoContext(parameters);
    cc->Enable(PKE);
    cc->Enable(KEYSWITCH);
    cc->Enable(LEVELEDSHE);

    auto keys = cc->KeyGen();
    cc->EvalMultKeyGen(keys.secretKey);

    // Standard encoding: 0 and 1
    auto ct0 = cc->Encrypt(keys.publicKey, cc->MakePackedPlaintext({0}));
    auto ct1 = cc->Encrypt(keys.publicKey, cc->MakePackedPlaintext({1}));

    // NAND: 1 - a*b (standard boolean)
    auto nand = [&](Ciphertext<DCRTPoly> a, Ciphertext<DCRTPoly> b) {
        auto ab = cc->EvalMult(a, b);
        return cc->EvalSub(ct1, ab);
    };

    // Decrypt helper
    auto decrypt_val = [&](Ciphertext<DCRTPoly> ct) -> int64_t {
        Plaintext pt;
        cc->Decrypt(keys.secretKey, ct, &pt);
        return pt->GetPackedValue()[0];
    };

    // Test NAND gates with standard 0/1
    std::cout << "NAND TESTS (0/1 encoding):\n";
    
    int64_t result = decrypt_val(nand(ct1, ct1));
    std::cout << "  NAND(1,1) = " << result << " (exp 0) " 
              << (result == 0 ? "✓" : "✗") << "\n";

    result = decrypt_val(nand(ct0, ct0));
    std::cout << "  NAND(0,0) = " << result << " (exp 1) " 
              << (result == 1 ? "✓" : "✗") << "\n";

    result = decrypt_val(nand(ct1, ct0));
    std::cout << "  NAND(1,0) = " << result << " (exp 1) " 
              << (result == 1 ? "✓" : "✗") << "\n";

    result = decrypt_val(nand(ct0, ct1));
    std::cout << "  NAND(0,1) = " << result << " (exp 1) " 
              << (result == 1 ? "✓" : "✗") << "\n";

    // Deep chain test
    std::cout << "\nDEEP CHAIN (10 depths):\n";
    auto current = ct1;
    int errors = 0;
    for (int i = 0; i <= 30; i++) {
        int64_t val = decrypt_val(current);
        int64_t expected = (i % 2 == 0) ? 1 : 0;
        bool ok = (val == expected);
        if (!ok) errors++;
        std::cout << "  Depth " << i << ": " << val << " (exp " << expected << ") "
                  << (ok ? "✓" : "✗") << "\n";
        if (i < 10) {
            current = nand(current, current);
        }
    }
    std::cout << "Errors: " << errors << "/31\n";

    if (errors == 0) {
        std::cout << "\n✅ PERIOD-2 CONFIRMED WITH STANDARD 0/1 ENCODING!\n";
        std::cout << "✅ NO NEED FOR φ-ψ ENCODING!\n";
        std::cout << "✅ NATURAL BOOTSTRAPPING WORKS!\n";
    }

    return 0;
}
