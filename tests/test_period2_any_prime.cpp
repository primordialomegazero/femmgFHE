// PERIOD-2 TEST: GUMAGANA BA SA ANY PRIME? (OpenFHE-compatible)
#include "openfhe.h"
#include <iostream>
#include <vector>

using namespace lbcrypto;

void test_prime(int p, int ring_dim, int depth) {
    std::cout << "TESTING p = " << p << " (ring " << ring_dim << ")\n";
    std::cout << "----------------------------------------\n";
    
    CCParams<CryptoContextBFVRNS> parameters;
    parameters.SetPlaintextModulus(p);
    parameters.SetMultiplicativeDepth(depth);
    parameters.SetRingDim(ring_dim);

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

    auto dec = [&](Ciphertext<DCRTPoly> ct) {
        Plaintext pt;
        cc->Decrypt(keys.secretKey, ct, &pt);
        return (int)pt->GetPackedValue()[0];
    };

    // Test period-2
    int errors = 0;
    for (int trials = 0; trials < 20; trials++) {
        auto ct = (trials % 2 == 0) ? ct0 : ct1;
        auto nand1 = nand(ct, ct);
        auto nand2 = nand(nand1, nand1);
        int result = dec(nand2);
        int expected = dec(ct);
        if (result != expected) {
            errors++;
        }
    }

    if (errors == 0) {
        std::cout << "  ✅ PERIOD-2 WORKS! (20/20)\n\n";
    } else {
        std::cout << "  ❌ " << errors << "/20 errors\n\n";
    }
}

int main() {
    std::cout << "====================================\n";
    std::cout << "  PERIOD-2 SA IBA'T IBANG PRIMES\n";
    std::cout << "====================================\n\n";

    // OpenFHE-compatible primes lamang
    test_prime(11, 16384, 10);      // May √5
    test_prime(17, 16384, 10);      // Fermat prime (WALANG √5)
    test_prime(257, 16384, 10);     // Fermat prime (WALANG √5)
    test_prime(65537, 32768, 20);   // Fermat prime (WALANG √5) — CURRENT

    return 0;
}
