// OPENFHE SIMPLE — Working Parameters
#include <NTL/ZZ.h>
#include "openfhe.h"
#include <iostream>
#include <vector>

using namespace lbcrypto;

int main() {
    std::cout << "========================================\n";
    std::cout << "  OPENFHE SIMPLE TEST\n";
    std::cout << "  Working Parameters\n";
    std::cout << "========================================\n\n";

    // Mas simpleng setup — gamitin ang defaults
    CCParams<CryptoContextBFVRNS> parameters;
    parameters.SetMultiplicativeDepth(10);
    parameters.SetPlaintextModulus(65537);

    CryptoContext<DCRTPoly> cc = GenCryptoContext(parameters);
    cc->Enable(PKE);
    cc->Enable(KEYSWITCH);
    cc->Enable(LEVELEDSHE);

    auto keys = cc->KeyGen();
    cc->EvalMultKeyGen(keys.secretKey);

    auto ring_dim = cc->GetRingDimension();
    auto plaintext_mod = cc->GetEncodingParams()->GetPlaintextModulus();

    std::cout << "BFV Context:\n";
    std::cout << "  Plaintext modulus: " << plaintext_mod << "\n";
    std::cout << "  Ring dimension: " << ring_dim << "\n";
    std::cout << "  Multiplicative depth: 10\n\n";

    // Simple test — diretsong NAND na walang φ-structure
    // I-verify muna kung basic homomorphic NAND ay gumagana

    std::vector<int64_t> vec_zero(ring_dim, 0);
    std::vector<int64_t> vec_one(ring_dim, 0);
    vec_one[0] = 1;

    auto ct_zero = cc->Encrypt(keys.publicKey, cc->MakePackedPlaintext(vec_zero));
    auto ct_one = cc->Encrypt(keys.publicKey, cc->MakePackedPlaintext(vec_one));

    // NAND: 1 - a·b
    auto homomorphic_nand = [&](Ciphertext<DCRTPoly> a, Ciphertext<DCRTPoly> b) {
        auto prod = cc->EvalMult(a, b);
        return cc->EvalSub(ct_one, prod);
    };

    auto decrypt_val = [&](Ciphertext<DCRTPoly> ct) {
        Plaintext pt;
        cc->Decrypt(keys.secretKey, ct, &pt);
        return pt->GetPackedValue()[0];
    };

    std::cout << "BASIC NAND TEST:\n";
    std::cout << "=================\n\n";

    std::cout << "  NAND(0,0) = " << decrypt_val(homomorphic_nand(ct_zero, ct_zero)) << " (expected 1)\n";
    std::cout << "  NAND(0,1) = " << decrypt_val(homomorphic_nand(ct_zero, ct_one)) << " (expected 1)\n";
    std::cout << "  NAND(1,0) = " << decrypt_val(homomorphic_nand(ct_one, ct_zero)) << " (expected 1)\n";
    std::cout << "  NAND(1,1) = " << decrypt_val(homomorphic_nand(ct_one, ct_one)) << " (expected 0)\n\n";

    // Deep chain
    std::cout << "DEEP CHAIN (10 gates):\n";
    std::cout << "=======================\n\n";

    auto current = ct_one;
    int errors = 0;

    for (int gate = 0; gate < 10; gate++) {
        current = homomorphic_nand(current, current);
        int got = decrypt_val(current);
        int expected = (gate % 2 == 0) ? 0 : 1;

        if (got != expected) errors++;

        std::cout << "  Gate " << gate << ": expected=" << expected
                  << " got=" << got
                  << (expected == got ? " YES" : " NO") << "\n";
    }

    std::cout << "\n  Result: " << errors << "/10 errors\n";

    return 0;
}
