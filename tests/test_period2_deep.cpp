// DEEP: Period-2 with higher multiplicative depth
#include "openfhe.h"
#include <iostream>

using namespace lbcrypto;

int main() {
    std::cout << "DEEP PERIOD-2 NAND (higher depth)\n";
    std::cout << "==================================\n\n";

    CCParams<CryptoContextBFVRNS> parameters;
    parameters.SetPlaintextModulus(65537);
    parameters.SetMultiplicativeDepth(30);  // INCREASED TO 30!
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

    std::cout << "DEEP CHAIN (30 depths):\n";
    auto current = ct1;
    int errors = 0;
    for (int i = 0; i <= 30; i++) {
        int64_t val = decrypt_val(current);
        int64_t expected = (i % 2 == 0) ? 1 : 0;
        bool ok = (val == expected);
        if (!ok) errors++;
        std::cout << "  Depth " << i << ": " << val << " (exp " << expected << ") "
                  << (ok ? "✓" : "✗") << "\n";
        if (i < 30) {
            current = nand(current, current);
        }
    }
    std::cout << "Errors: " << errors << "/31\n";

    if (errors == 0) {
        std::cout << "\n✅ PERIOD-2 CONFIRMED AT 30 DEPTHS!\n";
    } else {
        std::cout << "\n⚠️ Errors at depth " << errors << ". Increase parameters further.\n";
    }

    return 0;
}
