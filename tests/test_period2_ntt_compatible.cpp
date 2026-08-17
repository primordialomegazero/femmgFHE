// PERIOD-2 WITH NTT-COMPATIBLE MODULUS (65537)
#include "openfhe.h"
#include <iostream>

using namespace lbcrypto;

int main() {
    std::cout << "========================================\n";
    std::cout << "  PERIOD-2 WITH NTT-COMPATIBLE MODULUS  \n";
    std::cout << "  (65537 — Fermat prime, mabilis!)\n";
    std::cout << "========================================\n\n";

    CCParams<CryptoContextBFVRNS> parameters;
    parameters.SetPlaintextModulus(65537);  // NTT-compatible!
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

    auto decrypt_val = [&](Ciphertext<DCRTPoly> ct) -> int64_t {
        Plaintext pt;
        cc->Decrypt(keys.secretKey, ct, &pt);
        return pt->GetPackedValue()[0];
    };

    std::cout << "Testing period-2 with modulus = 65537\n";
    std::cout << "Depth | Decrypted | Expected | Status\n";
    std::cout << "------|-----------|----------|--------\n";

    auto current = ct1;
    int errors = 0;
    for (int i = 0; i <= 10; i++) {
        int64_t val = decrypt_val(current);
        int64_t expected = (i % 2 == 0) ? 1 : 0;
        bool ok = (val == expected);
        if (!ok) errors++;
        std::cout << i << " | " << val << " | " << expected << " | " 
                  << (ok ? "✓" : "✗") << "\n";
        if (i < 10) {
            current = nand(current, current);
        }
    }

    std::cout << "\nErrors: " << errors << "/11\n";
    if (errors == 0) {
        std::cout << "\n✅ PERIOD-2 WORKS WITH MODULUS = 65537!\n";
        std::cout << "✅ NTT-COMPATIBLE = FAST ARITHMETIC!\n";
        std::cout << "✅ FERMAT PRIME = BEST FOR BFV!\n";
    }

    return 0;
}
