// CKKS DEPTH 100 — Malalim na NAND chain
#include "openfhe.h"
#include <iostream>
#include <vector>
#include <complex>

using namespace lbcrypto;

int main() {
    std::cout << "========================================\n";
    std::cout << "  CKKS DEPTH 100 TEST\n";
    std::cout << "  Malalim na NAND chain\n";
    std::cout << "========================================\n\n";

    // High depth setup
    CCParams<CryptoContextCKKSRNS> parameters;
    parameters.SetMultiplicativeDepth(100);
    parameters.SetScalingModSize(50);
    parameters.SetBatchSize(8192);

    CryptoContext<DCRTPoly> cc = GenCryptoContext(parameters);
    cc->Enable(PKE);
    cc->Enable(KEYSWITCH);
    cc->Enable(LEVELEDSHE);

    auto keys = cc->KeyGen();
    cc->EvalMultKeyGen(keys.secretKey);

    auto ring_dim = cc->GetRingDimension();
    auto slots = cc->GetEncodingParams()->GetBatchSize();

    std::cout << "CKKS Context:\n";
    std::cout << "  Ring dimension: " << ring_dim << "\n";
    std::cout << "  Batch size: " << slots << "\n";
    std::cout << "  Depth: 100\n\n";

    // Binary values
    std::vector<std::complex<double>> vec_one(slots, {0.0, 0.0});
    vec_one[0] = {1.0, 0.0};

    std::vector<std::complex<double>> vec_zero(slots, {0.0, 0.0});

    auto ct_one_const = cc->Encrypt(keys.publicKey, cc->MakeCKKSPackedPlaintext(vec_one));
    auto ct_zero = cc->Encrypt(keys.publicKey, cc->MakeCKKSPackedPlaintext(vec_zero));

    // NAND: 1 - a·b
    auto std_nand = [&](Ciphertext<DCRTPoly> a, Ciphertext<DCRTPoly> b) {
        auto prod = cc->EvalMult(a, b);
        return cc->EvalSub(ct_one_const, prod);
    };

    auto decrypt_val = [&](Ciphertext<DCRTPoly> ct) {
        Plaintext pt;
        cc->Decrypt(keys.secretKey, ct, &pt);
        return pt->GetCKKSPackedValue()[0].real();
    };

    std::cout << "DEEP CHAIN (50 gates):\n";
    std::cout << "=======================\n\n";

    auto current = ct_one_const;
    int errors = 0;

    for (int gate = 0; gate < 50; gate++) {
        current = std_nand(current, current);

        double val = decrypt_val(current);
        int got = (std::abs(val) > 0.5) ? 1 : 0;
        int expected = (gate % 2 == 0) ? 0 : 1;

        if (got != expected) errors++;

        if (gate < 5 || gate >= 45 || (gate % 10 == 0)) {
            std::cout << "  Gate " << gate << ": val=" << val
                      << " expected=" << expected
                      << (got == expected ? " YES" : " NO") << "\n";
        }
    }

    std::cout << "\n  Result: " << errors << "/50 errors ("
              << (100.0 * (50 - errors) / 50) << "%)\n";

    return 0;
}
