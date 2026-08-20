// CKKS + φ SA PLAINTEXT — DEPTH 60
// Mas malalim na chain para ma-verify ang period-2 stability

#include "openfhe.h"
#include <iostream>
#include <vector>
#include <complex>

using namespace lbcrypto;

int main() {
    std::cout << "========================================\n";
    std::cout << "  CKKS + φ — DEPTH 60\n";
    std::cout << "  Period-2 Stability Test\n";
    std::cout << "========================================\n\n";

    // Depth 60, scaling 40
    CCParams<CryptoContextCKKSRNS> params;
    params.SetMultiplicativeDepth(60);
    params.SetScalingModSize(40);
    params.SetBatchSize(2048);

    CryptoContext<DCRTPoly> cc = GenCryptoContext(params);
    cc->Enable(PKE);
    cc->Enable(KEYSWITCH);
    cc->Enable(LEVELEDSHE);

    auto keys = cc->KeyGen();
    cc->EvalMultKeyGen(keys.secretKey);

    auto slots = cc->GetEncodingParams()->GetBatchSize();
    auto ring = cc->GetRingDimension();

    std::cout << "CKKS: ring=" << ring << ", slots=" << slots << ", depth=60\n\n";

    // φ-Structure
    const double phi_val = 1.6180339887498948482;
    const double inv_phi = 1.0 / phi_val;

    std::vector<std::complex<double>> vec_phi(slots, {0.0, 0.0});
    vec_phi[0] = {phi_val, 0.0};

    std::vector<std::complex<double>> vec_inv_phi(slots, {0.0, 0.0});
    vec_inv_phi[0] = {inv_phi, 0.0};

    std::vector<std::complex<double>> vec_zero(slots, {0.0, 0.0});

    auto ct_phi = cc->Encrypt(keys.publicKey, cc->MakeCKKSPackedPlaintext(vec_phi));
    auto ct_inv_phi = cc->Encrypt(keys.publicKey, cc->MakeCKKSPackedPlaintext(vec_inv_phi));
    auto ct_zero = cc->Encrypt(keys.publicKey, cc->MakeCKKSPackedPlaintext(vec_zero));
    auto ct_one = ct_phi;

    auto phi_nand = [&](auto a, auto b) {
        auto prod = cc->EvalMult(a, b);
        auto scaled = cc->EvalMult(prod, ct_inv_phi);
        return cc->EvalSub(ct_phi, scaled);
    };

    auto decrypt_val = [&](auto ct) {
        Plaintext pt;
        cc->Decrypt(keys.secretKey, ct, &pt);
        return pt->GetCKKSPackedValue()[0].real();
    };

    // ============================================
    // DEEP CHAIN: 28 gates (56 multiplications)
    // ============================================
    std::cout << "DEEP CHAIN: 28 gates (56 mults)\n";
    std::cout << "===================================\n\n";

    auto current = ct_one;
    int errors = 0;

    std::cout << "  Gate | Value | Expected | OK?\n";
    std::cout << "  -----|-------|----------|-----\n";

    for (int gate = 0; gate < 28; gate++) {
        current = phi_nand(current, current);

        double v = decrypt_val(current);
        double expected = (gate % 2 == 0) ? 0.0 : phi_val;
        bool ok = (std::abs(v - expected) < 0.1);

        if (!ok) errors++;

        // Print every 4 gates para hindi masyadong mahaba
        if (gate < 8 || gate >= 24) {
            std::cout << "  " << gate << "    | " << v
                      << " | " << expected
                      << " | " << (ok ? "YES" : "NO") << "\n";
        }
    }

    std::cout << "\n  Result: " << errors << "/28 errors ("
              << (100.0 * (28 - errors) / 28) << "%)\n\n";

    // ============================================
    // NOISE TRACE — May exponential growth ba?
    // ============================================
    std::cout << "NOISE TRACE (even gates — dapat ≈ 0):\n";
    std::cout << "========================================\n\n";

    current = ct_one;
    for (int gate = 0; gate < 28; gate++) {
        current = phi_nand(current, current);
        if (gate % 2 == 0) {
            double v = decrypt_val(current);
            std::cout << "  Gate " << gate << ": noise = " << std::abs(v) << "\n";
        }
    }

    return 0;
}
