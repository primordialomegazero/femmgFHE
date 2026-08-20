// CKKS + φ-STRUCTURE + BOOTSTRAPPING — FIXED
#include "openfhe.h"
#include <iostream>
#include <vector>
#include <complex>

using namespace lbcrypto;

int main() {
    std::cout << "========================================\n";
    std::cout << "  CKKS + φ-STRUCTURE + BOOTSTRAPPING\n";
    std::cout << "========================================\n\n";

    // CKKS Setup
    CCParams<CryptoContextCKKSRNS> parameters;
    parameters.SetMultiplicativeDepth(10);
    parameters.SetScalingModSize(50);
    parameters.SetBatchSize(8192);

    CryptoContext<DCRTPoly> cc = GenCryptoContext(parameters);
    cc->Enable(PKE);
    cc->Enable(KEYSWITCH);
    cc->Enable(LEVELEDSHE);
    cc->Enable(ADVANCEDSHE);
    cc->Enable(FHE);

    auto keys = cc->KeyGen();
    cc->EvalMultKeyGen(keys.secretKey);

    // Bootstrapping setup na may default parameters
    std::vector<uint32_t> levelBudget = {5, 4};
    std::vector<uint32_t> dim1 = {0, 0};
    uint32_t slots = 8192;
    uint32_t correctionFactor = 1;
    bool precompute = true;

    cc->EvalBootstrapSetup(levelBudget, dim1, slots, correctionFactor, precompute);
    cc->EvalBootstrapKeyGen(keys.secretKey, slots);

    auto ring_dim = cc->GetRingDimension();

    std::cout << "CKKS Context:\n";
    std::cout << "  Ring dimension: " << ring_dim << "\n";
    std::cout << "  Slots: " << slots << "\n\n";

    // φ-Structure sa CKKS
    const double phi_val = 1.6180339887498948482;
    const double phi_k_val = std::pow(phi_val, 42.0);
    const double inv_phi_k_val = 1.0 / phi_k_val;

    std::cout << "φ^k ≈ " << phi_k_val << "\n";
    std::cout << "φ^(-k) ≈ " << inv_phi_k_val << "\n\n";

    // Encode
    std::vector<std::complex<double>> vec_phi_k(slots, {0.0, 0.0});
    vec_phi_k[0] = {phi_k_val, 0.0};

    std::vector<std::complex<double>> vec_inv_phi_k(slots, {0.0, 0.0});
    vec_inv_phi_k[0] = {inv_phi_k_val, 0.0};

    std::vector<std::complex<double>> vec_zero(slots, {0.0, 0.0});

    auto ct_phi_k = cc->Encrypt(keys.publicKey, cc->MakeCKKSPackedPlaintext(vec_phi_k));
    auto ct_inv_phi_k = cc->Encrypt(keys.publicKey, cc->MakeCKKSPackedPlaintext(vec_inv_phi_k));
    auto ct_zero = cc->Encrypt(keys.publicKey, cc->MakeCKKSPackedPlaintext(vec_zero));
    auto ct_one = ct_phi_k;

    std::cout << "Ciphertexts ready\n\n";

    // φ-NAND
    auto phi_nand = [&](Ciphertext<DCRTPoly> a, Ciphertext<DCRTPoly> b) {
        auto prod = cc->EvalMult(a, b);
        auto scaled = cc->EvalMult(prod, ct_inv_phi_k);
        return cc->EvalSub(ct_phi_k, scaled);
    };

    auto decrypt_val = [&](Ciphertext<DCRTPoly> ct) {
        Plaintext pt;
        cc->Decrypt(keys.secretKey, ct, &pt);
        return pt->GetCKKSPackedValue()[0].real();
    };

    auto is_one = [&](double val) {
        return std::abs(val - phi_k_val) < std::abs(val);
    };

    // Truth table
    std::cout << "φ-NAND TRUTH TABLE:\n";
    std::cout << "=====================\n\n";

    auto nand_00 = phi_nand(ct_zero, ct_zero);
    auto nand_01 = phi_nand(ct_zero, ct_one);
    auto nand_10 = phi_nand(ct_one, ct_zero);
    auto nand_11 = phi_nand(ct_one, ct_one);

    std::cout << "  NAND(0,0) = " << decrypt_val(nand_00) << "\n";
    std::cout << "  NAND(0,1) = " << decrypt_val(nand_01) << "\n";
    std::cout << "  NAND(1,0) = " << decrypt_val(nand_10) << "\n";
    std::cout << "  NAND(1,1) = " << decrypt_val(nand_11) << "\n\n";

    // Deep chain
    std::cout << "DEEP CHAIN (20 gates) na may Bootstrapping every 5:\n";
    std::cout << "=====================================================\n\n";

    auto current = ct_one;
    int errors = 0;

    for (int gate = 0; gate < 20; gate++) {
        current = phi_nand(current, current);

        double val = decrypt_val(current);
        int got = is_one(val) ? 1 : 0;
        int expected = (gate % 2 == 0) ? 0 : 1;

        if (got != expected) errors++;

        if (gate < 5 || gate >= 15) {
            std::cout << "  Gate " << gate << ": val=" << val
                      << " expected=" << expected
                      << (got == expected ? " YES" : " NO") << "\n";
        }

        // Bootstrapping tuwing 5 gates
        if (gate % 5 == 4) {
            current = cc->EvalBootstrap(current);
        }
    }

    std::cout << "\n  Result: " << errors << "/20 errors ("
              << (100.0 * (20 - errors) / 20) << "%)\n";

    return 0;
}
