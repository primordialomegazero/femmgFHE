// CKKS + φ-STRUCTURE — Scaled para sa precision
#include "openfhe.h"
#include <iostream>
#include <vector>
#include <complex>

using namespace lbcrypto;

int main() {
    std::cout << "========================================\n";
    std::cout << "  CKKS + φ-STRUCTURE (Scaled)\n";
    std::cout << "  Binary encoding na may maliit na values\n";
    std::cout << "========================================\n\n";

    // CKKS Setup
    CCParams<CryptoContextCKKSRNS> parameters;
    parameters.SetMultiplicativeDepth(15);
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
    std::cout << "  Batch size: " << slots << "\n\n";

    // SCALED φ-STRUCTURE
    // Sa halip na φ^k (malaking value), gamitin natin
    // ang binary encoding na 0 at 1
    // NAND(a,b) = 1 - a·b

    std::vector<std::complex<double>> vec_one(slots, {0.0, 0.0});
    vec_one[0] = {1.0, 0.0};

    std::vector<std::complex<double>> vec_zero(slots, {0.0, 0.0});

    auto ct_one_const = cc->Encrypt(keys.publicKey, cc->MakeCKKSPackedPlaintext(vec_one));
    auto ct_zero = cc->Encrypt(keys.publicKey, cc->MakeCKKSPackedPlaintext(vec_zero));
    auto ct_one = ct_one_const;

    // Standard NAND: 1 - a·b
    auto std_nand = [&](Ciphertext<DCRTPoly> a, Ciphertext<DCRTPoly> b) {
        auto prod = cc->EvalMult(a, b);
        return cc->EvalSub(ct_one_const, prod);
    };

    auto decrypt_val = [&](Ciphertext<DCRTPoly> ct) {
        Plaintext pt;
        cc->Decrypt(keys.secretKey, ct, &pt);
        return pt->GetCKKSPackedValue()[0].real();
    };

    std::cout << "STANDARD NAND TRUTH TABLE:\n";
    std::cout << "============================\n\n";

    std::cout << "  NAND(0,0) = " << decrypt_val(std_nand(ct_zero, ct_zero)) << " (expected 1)\n";
    std::cout << "  NAND(0,1) = " << decrypt_val(std_nand(ct_zero, ct_one)) << " (expected 1)\n";
    std::cout << "  NAND(1,0) = " << decrypt_val(std_nand(ct_one, ct_zero)) << " (expected 1)\n";
    std::cout << "  NAND(1,1) = " << decrypt_val(std_nand(ct_one, ct_one)) << " (expected 0)\n\n";

    // Deep chain
    std::cout << "DEEP CHAIN (20 gates):\n";
    std::cout << "=======================\n\n";

    auto current = ct_one;
    int errors = 0;

    for (int gate = 0; gate < 20; gate++) {
        current = std_nand(current, current);

        double val = decrypt_val(current);
        int got = (std::abs(val) > 0.5) ? 1 : 0;
        int expected = (gate % 2 == 0) ? 0 : 1;

        if (got != expected) errors++;

        std::cout << "  Gate " << gate << ": val=" << val
                  << " expected=" << expected
                  << (got == expected ? " YES" : " NO") << "\n";
    }

    std::cout << "\n  Result: " << errors << "/20 errors ("
              << (100.0 * (20 - errors) / 20) << "%)\n";

    return 0;
}
