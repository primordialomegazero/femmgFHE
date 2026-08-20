// CKKS BOOTSTRAPPING — Working Setup
#include "openfhe.h"
#include <iostream>
#include <vector>
#include <complex>

using namespace lbcrypto;

int main() {
    std::cout << "========================================\n";
    std::cout << "  CKKS BOOTSTRAPPING TEST\n";
    std::cout << "========================================\n\n";

    // CKKS na may bootstrapping — tamang setup
    CCParams<CryptoContextCKKSRNS> parameters;
    parameters.SetMultiplicativeDepth(20);
    parameters.SetScalingModSize(59);
    parameters.SetFirstModSize(60);
    parameters.SetBatchSize(4096);

    CryptoContext<DCRTPoly> cc = GenCryptoContext(parameters);
    cc->Enable(PKE);
    cc->Enable(KEYSWITCH);
    cc->Enable(LEVELEDSHE);
    cc->Enable(ADVANCEDSHE);
    cc->Enable(FHE);

    auto keys = cc->KeyGen();
    cc->EvalMultKeyGen(keys.secretKey);

    // Bootstrapping setup
    uint32_t slots = 4096;
    std::vector<uint32_t> levelBudget = {7, 7};
    std::vector<uint32_t> dim1 = {0, 0};

    try {
        cc->EvalBootstrapSetup(levelBudget, dim1, slots);
        cc->EvalBootstrapKeyGen(keys.secretKey, slots);
        std::cout << "Bootstrapping setup: OK\n\n";
    } catch (std::exception& e) {
        std::cout << "Bootstrapping setup FAILED: " << e.what() << "\n";
        std::cout << "Trying alternative setup...\n\n";

        // Alternative: mas mababang slots
        try {
            slots = 2048;
            cc->EvalBootstrapSetup(levelBudget, dim1, slots);
            cc->EvalBootstrapKeyGen(keys.secretKey, slots);
            std::cout << "Alternative bootstrapping setup: OK\n\n";
        } catch (std::exception& e2) {
            std::cout << "Alternative also FAILED: " << e2.what() << "\n";
            return 1;
        }
    }

    // Simple binary values
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

    std::cout << "DEEP CHAIN (30 gates) WITH BOOTSTRAPPING:\n";
    std::cout << "===========================================\n\n";

    auto current = ct_one_const;
    int errors = 0;

    for (int gate = 0; gate < 30; gate++) {
        current = std_nand(current, current);

        double val = decrypt_val(current);
        int got = (std::abs(val) > 0.5) ? 1 : 0;
        int expected = (gate % 2 == 0) ? 0 : 1;

        if (got != expected) errors++;

        if (gate < 5 || gate >= 25) {
            std::cout << "  Gate " << gate << ": val=" << val
                      << " expected=" << expected
                      << (got == expected ? " YES" : " NO") << "\n";
        }

        // Bootstrapping every 5 gates
        if (gate % 5 == 4) {
            try {
                current = cc->EvalBootstrap(current);
            } catch (std::exception& e) {
                std::cout << "  Bootstrap FAILED at gate " << gate << ": " << e.what() << "\n";
                break;
            }
        }
    }

    std::cout << "\n  Result: " << errors << "/30 errors\n";

    return 0;
}
